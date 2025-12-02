#include "Network.h"
#include "netcode.h"
#include "main.h"
#include "Thread.h"
#include "Log.h"
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include "main.h"
#include "RawNetMessage.h"
#include "ANSIColourCodes.h"
#include "ThreadSafeQueue.h"
#include "ObjectPool.h"
#include "DynArray.h"
#include "SharedPtr.h"
#include "AssertLib.h"

struct ReliableMessageTracker
{
    u32 ident;
    SHARED_PTR(void) data;
    u32 dataSize;
        
    struct ReliableMessageTracker* pNext;
    struct ReliableMessageTracker* pPrev;
};

struct MessageFragment
{
    u8* pData;
    u32 size;
};

struct FragmentedMessage
{
    u8* pData;
    u32 dataSize;
    VECTOR(struct MessageFragment) pFragments;
};

typedef HGeneric HReliableTracker;

static OBJECT_POOL(struct ReliableMessageTracker) gReliableTrackerPool = NULL;

struct ReliableMessageTracker* pReliableTrackerListHead = NULL;
struct ReliableMessageTracker* pReliableTrackerListTail = NULL;

static enum GameRole gRole;

static struct NetworkThreadQueues* pQueues;

#define TEST_PROTOCOL_ID 0x1122334455667788
#define GAME_PROTOCOL_ID TEST_PROTOCOL_ID
#define CONNECT_TOKEN_EXPIRY 30
#define CONNECT_TOKEN_TIMEOUT 5
#define GAME_MAX_CLIENTS 3


enum GameClientState
{
    GCS_Disconnected,
    GCS_WaitingToPlay,
    GCS_Playing
};

struct GameClient
{
    enum GameClientState state;
};

#define RECENTLY_ACKED_PACKETS_BUF_SIZE 64
static u32 gRecentlyAckedIdentBufferSize = 0;
static u32 gRecentlyAckedPacketIdentifiers[RECENTLY_ACKED_PACKETS_BUF_SIZE];

static void PushAckedPacketIdentifier(u32 ident)
{
    static int nextI = -1;
    gRecentlyAckedPacketIdentifiers[nextI++] = ident;
    if(nextI >= RECENTLY_ACKED_PACKETS_BUF_SIZE)
    {
        nextI = 0;
    }
    gRecentlyAckedIdentBufferSize++;
}

int clamp(int i, int c)
{
    return i <= c ? i : c;
}

static bool HasReliablePacketBeenRecentlyAcknowledged(u32 ident)
{
    for(int i=0; i<clamp(gRecentlyAckedIdentBufferSize, RECENTLY_ACKED_PACKETS_BUF_SIZE); i++)
    {
        if(ident == gRecentlyAckedPacketIdentifiers[i])
        {
            return true;
        }
    }
    return false;
}


static u8 gPacketBuffer[NETCODE_MAX_PACKET_SIZE];

CrossPlatformThread gNetworkThread;

static uint8_t private_key[NETCODE_KEY_BYTES] = { 0x60, 0x6a, 0xbe, 0x6e, 0xc9, 0x19, 0x10, 0xea, 
                                                  0x9a, 0x65, 0x62, 0xf6, 0x6f, 0x2b, 0x30, 0xe4, 
                                                  0x43, 0x71, 0xd6, 0x2c, 0xd1, 0x99, 0x27, 0x26,
                                                  0x6b, 0x3c, 0x60, 0xf4, 0xb7, 0x15, 0xab, 0xa1 };


static int NetcodeLog(const char* fmt, ...)
{
    static char* netcodeLogTagString = "[Netcode] ";
    static char* netcodeColouredLogTagString = BHBLU"[Netcode]"CRESET" ";
    char buf[512];
    va_list args;
    va_start(args, fmt);
    
    sprintf(buf, gCmdArgs.bLogTextColoured ? netcodeColouredLogTagString : netcodeLogTagString);
    int len = strlen(buf);
    vsnprintf(buf + len, 512 - len, fmt, args);
    len = strlen(buf);
    while(buf[len - 1] == '\n' || buf[len - 1] == '\r')
    {
        buf[--len] = '\0';
    }
    
    Log_Info(buf);
    va_end(args);
}

static void InitClients(struct GameClient* clients)
{
    for(int i=0; i < GAME_MAX_CLIENTS; i++)
    {
        clients[i].state = GCS_Disconnected;
    }
}

static void ServiceClientConnectionEvents(struct GameClient* client, struct netcode_server_t * server, int clientIndex, struct NetworkThreadQueues* pQueues)
{
    if(!netcode_server_client_connected( server, clientIndex ))
    {
        if(client->state != GCS_Disconnected)
        {
            struct NetworkConnectionEvent event = 
            {
                .client = clientIndex,
                .type = NCE_ClientDisconnected
            };
            TSQ_Enqueue(&pQueues->connectionEvents, &event);
        }
        client->state = GCS_Disconnected;
    }
    else
    {
        switch(client->state)
        {
        case GCS_Disconnected:
            {
                struct NetworkConnectionEvent event = 
                {
                    .client = clientIndex,
                    .type = NCE_ClientConnected
                };
                TSQ_Enqueue(&pQueues->connectionEvents, &event);
                client->state = GCS_WaitingToPlay;
            }
            break;
        case GCS_WaitingToPlay:
            break;
        case GCS_Playing:
            break;
        }
    }
    
}


DECLARE_THREAD_PROC(ClientThread, arg)
{
    

    netcode_set_printf_function(&NetcodeLog);
    if ( netcode_init() != NETCODE_OK )
    {
        Log_Error( "error: failed to initialize netcode" );
        return (void*)1;
    }

    netcode_log_level( NETCODE_LOG_LEVEL_INFO );

    double time = 0.0;
    double delta_time = 1.0 / 60.0;

    Log_Info( "client" );

    struct netcode_client_config_t client_config;
    netcode_default_client_config( &client_config );
    struct netcode_client_t * client = netcode_client_create( "0.0.0.0", &client_config, time );

    if ( !client )
    {
        Log_Error( "error: failed to create client" );
        return (void*)1;
    }

    NETCODE_CONST char* server_address = gCmdArgs.serverAddress;

    uint64_t client_id = 0;
    netcode_random_bytes( (uint8_t*) &client_id, 8 );
    Log_Info( "client id is %.16" PRIx64 , client_id );

    uint8_t user_data[NETCODE_USER_DATA_BYTES];
    netcode_random_bytes(user_data, NETCODE_USER_DATA_BYTES);

    uint8_t connect_token[NETCODE_CONNECT_TOKEN_BYTES];

    if ( netcode_generate_connect_token( 1, &server_address, &server_address, CONNECT_TOKEN_EXPIRY, CONNECT_TOKEN_TIMEOUT, client_id, GAME_PROTOCOL_ID, private_key, user_data, connect_token ) != NETCODE_OK )
    {
        Log_Error( "error: failed to generate connect token" );
        return (void*)1;
    }

    netcode_client_connect( client, connect_token );

    bool quit = false;

    while ( !quit )
    {
        netcode_client_update( client, time );

        netcode_sleep( delta_time );

        time += delta_time;
    }

    if ( quit )
    {
        Log_Info( "shutting netcode thread down" );
    }

    netcode_client_destroy( client );

    netcode_term();
    return NULL;
}

static void AcknowledgeIdentifier(u32 ident)
{
    struct ReliableMessageTracker* pTracker = pReliableTrackerListHead;
    while(pTracker)
    {
        if(pTracker->ident == ident)
        {
            if(pReliableTrackerListHead == pTracker)
            {
                pReliableTrackerListHead = pTracker->pNext;
            }
            if(pReliableTrackerListTail == pTracker)
            {
                pReliableTrackerListTail = pTracker->pPrev;
            }
            if(pTracker->pNext)
            {
                pTracker->pNext->pPrev = pTracker->pPrev;
            }
            if(pTracker->pPrev)
            {
                pTracker->pPrev->pNext = pTracker->pNext;
            }
            Sptr_RemoveRef(pTracker->data);
            return;
        }
        pTracker = pTracker->pNext;
    }
    PushAckedPacketIdentifier(ident);
    Log_Info("Reliable message ack packet recieved, doesn't correspond to any in list. ID: %i", ident);
}

static void ServerRecievePackets(struct netcode_server_t * server, struct NetworkThreadQueues* pQueues)
{
    int client_index;
    for ( client_index = 0; client_index < NETCODE_MAX_CLIENTS; ++client_index )
    {
        while ( 1 )             
        {
            int packet_bytes;
            uint64_t packet_sequence;
            void * packet = netcode_server_receive_packet( server, client_index, &packet_bytes, &packet_sequence );
            if ( !packet )
                break;

            /* respond to packet here */
            enum NetRawMessageType msgType;
            u8 pBody = NULL;
            NetMsg_Parse(packet, &msgType, &pBody);
            int headersSize = NetMsg_SizeOfHeaders(msgType);
            int payloadSize = packet_bytes - headersSize;
            switch (msgType)
            {
            case UnreliableDataMessageComplete:
                {
                    
                    struct NetworkQueueItem qItem;
                    qItem.client = client_index;
                    qItem.pData = malloc(payloadSize);
                    qItem.pDataSize = payloadSize;
                    memcpy(qItem.pData, pBody, payloadSize);
                    qItem.bReliable = false;
                    TSQ_Enqueue(&pQueues->rx, &qItem);
                }
                break;
            case ReliableDataMessageComplete:
                {
                    /* acknowledge */
                    /*
                        idea:
                        keep a circular buffer of recently acked packets, search it here before proceeding.
                        A crude filter.
                    */
                    struct NetReliableMessageHeader* pHeader = NetMsg_GetReliableHeader(packet);
                    if(!HasReliablePacketBeenRecentlyAcknowledged(pHeader->messageIdentifier))
                    {
                        int packetSize = NetMsg_WriteReliableDataAckPacket(gPacketBuffer, pHeader->messageIdentifier);
                        netcode_server_send_packet(server, client_index, gPacketBuffer, packetSize);

                        struct NetworkQueueItem qItem;
                        qItem.client = client_index;
                        qItem.pData = malloc(payloadSize);
                        qItem.pDataSize = payloadSize;
                        memcpy(qItem.pData, pBody, payloadSize);
                        qItem.bReliable = false;
                        TSQ_Enqueue(&pQueues->rx, &qItem);
                    }
                    else
                    {
                        int packetSize = NetMsg_WriteReliableDataAckPacket(gPacketBuffer, pHeader->messageIdentifier);
                        netcode_server_send_packet(server, client_index, gPacketBuffer, packetSize);
                    }
                    
                }
                break;
            case ReliableDataMessageFragment:
                break;
            case ReliableDataMessageAck:
                {
                    u32 identifier = NetMsg_GetAckedIdentifier(packet);
                    AcknowledgeIdentifier(identifier);
                }
                break;
            }

            netcode_server_free_packet( server, packet );
        }
    }

}

static u32 TrackReliableMessage(u8* data, u32 dataSize)
{
    HReliableTracker hT;
    gReliableTrackerPool = GetObjectPoolIndex(gReliableTrackerPool, &hT);
    gReliableTrackerPool[hT].ident = NetMsg_GetReliableMessageIdentifier();
    gReliableTrackerPool[hT].data = data;
    gReliableTrackerPool[hT].dataSize = dataSize;
    gReliableTrackerPool[hT].pNext = NULL;
    gReliableTrackerPool[hT].pPrev = NULL;

    if(pReliableTrackerListHead == NULL)
    {
        pReliableTrackerListHead = &gReliableTrackerPool[hT];
        pReliableTrackerListTail = pReliableTrackerListHead;
        return gReliableTrackerPool[hT].ident;
    }
    pReliableTrackerListTail->pNext = &gReliableTrackerPool[hT];
    gReliableTrackerPool[hT].pPrev = pReliableTrackerListTail;
    pReliableTrackerListTail = &gReliableTrackerPool[hT];
    return gReliableTrackerPool[hT].ident;
}

static void SendMessageFragments(struct NetworkThreadQueues* pQueues, struct netcode_server_t* server, struct NetworkQueueItem* item)
{
    EASSERT(item->pDataSize > NETCODE_MAX_PACKET_SIZE);
    // TODO: IMPLEMENT ME NEXT
    
}

static void DoTXQueue(struct NetworkThreadQueues* pQueues, struct netcode_server_t* server)
{
    struct NetworkQueueItem item;
    while(TSQ_Dequeue(&pQueues->tx, &item))
    {
        if(item.pDataSize > NETCODE_MAX_PACKET_SIZE)
        {
            /* send fragments here */
            SendMessageFragments(pQueues, server, &item);
            Sptr_RemoveRef(item.pData);
        }
        else
        {
            if(item.bReliable)
            {
                int packetSize = NetMsg_WriteReliableCompleteDataPacket(gPacketBuffer, item.pData, item.pDataSize, TrackReliableMessage(item.pData, item.pDataSize));
                netcode_server_send_packet(server, item.client, gPacketBuffer, packetSize);
                continue;
            }
            else
            {
                int packetSize = NetMsg_WriteUnreliableCompleteDataPacket(gPacketBuffer, item.pData, item.pDataSize);
                netcode_server_send_packet(server, item.client, gPacketBuffer, packetSize);
                Sptr_RemoveRef(item.pData);
            }
        }
        
    }
}

DECLARE_THREAD_PROC(ClientServerThread, arg)
{

    struct GameClient gameClients[GAME_MAX_CLIENTS];
    struct NetworkThreadQueues* pQueues = arg; 
    InitClients(&gameClients[0]);
    netcode_set_printf_function(&NetcodeLog);
    if ( netcode_init() != NETCODE_OK )
    {
        Log_Error( "failed to initialize netcode" );
        return (void*)1;
    }

    netcode_log_level( NETCODE_LOG_LEVEL_INFO );

    double time = 0.0;
    double delta_time = 1.0 / 60.0;

    Log_Info( "[server]" );

    NETCODE_CONST char* server_address = gCmdArgs.serverAddress;
    struct netcode_server_config_t server_config;
    netcode_default_server_config( &server_config );
    server_config.protocol_id = GAME_PROTOCOL_ID;
    memcpy( &server_config.private_key, private_key, NETCODE_KEY_BYTES );

    struct netcode_server_t * server = netcode_server_create( server_address, &server_config, time );

    if ( !server )
    {
        Log_Info( "error: failed to create server" );
        return (void*)1;
    }

    netcode_server_start( server, GAME_MAX_CLIENTS );
    bool quit = false;
    while ( !quit )
    {
        netcode_server_update( server, time );
        
        for(int i=0; i<GAME_MAX_CLIENTS; i++)
        {
            ServiceClientConnectionEvents(&gameClients[i], server, i, pQueues);
        }

        DoTXQueue(pQueues, server);

        ServerRecievePackets(server, pQueues);

        netcode_sleep( delta_time );

        time += delta_time;
    }

    if ( quit )
    {
        Log_Info( "shutting netcode thread down" );
    }

    netcode_server_destroy( server );

    netcode_term();
}

void WrapAroundHandlerBase(void* pItemToBeLost, const char* message)
{
    struct NetworkQueueItem* pItem = pItemToBeLost;
    Sptr_RemoveRef(pItem->pData);
    Log_Warning(message);
}


void OnConnectionEventTSQueueWrapAround(void* pItemToBeLost)
{
    WrapAroundHandlerBase(pItemToBeLost, "Network thread Connection event queue wrapped around, packets lost. It must not have been emptied quick enough");
}

void OnTXTSQueueWrapAround(void* pItemToBeLost)
{
    WrapAroundHandlerBase(pItemToBeLost, "Network thread TX queue wrapped around, packets lost. It must not have been emptied quick enough");
}

void OnRXTSQueueWrapAround(void* pItemToBeLost)
{
    WrapAroundHandlerBase(pItemToBeLost, "Network thread RX queue wrapped around, packets lost. It must not have been emptied quick enough");
}

void NW_Init()
{
    gReliableTrackerPool = NEW_OBJECT_POOL(struct ReliableMessageTracker, 128);
    gRole = gCmdArgs.role;

    switch(gRole)
    {
    case GR_Client:
    case GR_ClientServer:

        pQueues = malloc(sizeof(struct NetworkThreadQueues));
        memset(pQueues, 0, sizeof(struct NetworkThreadQueues));
        TSQ_Init(&pQueues->rx, sizeof(struct NetworkQueueItem), 32, &OnRXTSQueueWrapAround);
        TSQ_Init(&pQueues->tx, sizeof(struct NetworkQueueItem), 32, &OnTXTSQueueWrapAround);
        TSQ_Init(&pQueues->connectionEvents, sizeof(struct NetworkConnectionEvent), 32, OnConnectionEventTSQueueWrapAround);
    }
    switch (gRole)
    {
    case GR_Client:
        gNetworkThread = StartThread(&ClientThread, pQueues);
        break;
    case GR_ClientServer:
        gNetworkThread = StartThread(&ClientServerThread, pQueues);
        break;
    }
}

