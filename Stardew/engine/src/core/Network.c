#include "Network.h"
#include "netcode.h"
#include "main.h"
#include "Thread.h"
#include "Log.h"
#include <stdbool.h>

enum GameRole gRole;

#define TEST_PROTOCOL_ID 0x1122334455667788
#define GAME_PROTOCOL_ID TEST_PROTOCOL_ID

CrossPlatformThread gNetworkThread;

static uint8_t private_key[NETCODE_KEY_BYTES] = { 0x60, 0x6a, 0xbe, 0x6e, 0xc9, 0x19, 0x10, 0xea, 
                                                  0x9a, 0x65, 0x62, 0xf6, 0x6f, 0x2b, 0x30, 0xe4, 
                                                  0x43, 0x71, 0xd6, 0x2c, 0xd1, 0x99, 0x27, 0x26,
                                                  0x6b, 0x3c, 0x60, 0xf4, 0xb7, 0x15, 0xab, 0xa1 };


DECLARE_THREAD_PROC(ClientThread, arg)
{

}

DECLARE_THREAD_PROC(ClientServerThread, arg)
{
    netcode_set_printf_function(&Log_Info);
    if ( netcode_init() != NETCODE_OK )
    {
        Log_Error( "failed to initialize netcode\n" );
        return 1;
    }

    netcode_log_level( NETCODE_LOG_LEVEL_INFO );

    double time = 0.0;
    double delta_time = 1.0 / 60.0;

    Log_Info( "[server]\n" );

    char* server_address = gCmdArgs.serverAddress;
    struct netcode_server_config_t server_config;
    netcode_default_server_config( &server_config );
    server_config.protocol_id = GAME_PROTOCOL_ID;
    memcpy( &server_config.private_key, private_key, NETCODE_KEY_BYTES );

    struct netcode_server_t * server = netcode_server_create( server_address, &server_config, time );

    if ( !server )
    {
        Log_Info( "error: failed to create server\n" );
        return 1;
    }

    netcode_server_start( server, NETCODE_MAX_CLIENTS );
    bool quit = false;
    while ( !quit )
    {
        netcode_server_update( server, time );

        // if ( netcode_server_client_connected( server, 0 ) )
        // {
        //     netcode_server_send_packet( server, 0, packet_data, NETCODE_MAX_PACKET_SIZE );
        // }

        // int client_index;
        // for ( client_index = 0; client_index < NETCODE_MAX_CLIENTS; ++client_index )
        // {
        //     while ( 1 )             
        //     {
        //         int packet_bytes;
        //         uint64_t packet_sequence;
        //         void * packet = netcode_server_receive_packet( server, client_index, &packet_bytes, &packet_sequence );
        //         if ( !packet )
        //             break;
        //         (void) packet_sequence;
        //         assert( packet_bytes == NETCODE_MAX_PACKET_SIZE );
        //         assert( memcmp( packet, packet_data, NETCODE_MAX_PACKET_SIZE ) == 0 );            
        //         netcode_server_free_packet( server, packet );
        //     }
        // }

        netcode_sleep( delta_time );

        time += delta_time;
    }

    if ( quit )
    {
        Log_Info( "shutting netcode thread down\n" );
    }

    netcode_server_destroy( server );

    netcode_term();
}

void NW_Init()
{
    gRole = gCmdArgs.role;
    switch(gRole)
    {
    case GR_Client:
        gNetworkThread = StartThread(ClientThread, NULL);
        break;
    case GR_ClientServer:
        gNetworkThread = StartThread(ClientServerThread, NULL);
        break;
    }
}

