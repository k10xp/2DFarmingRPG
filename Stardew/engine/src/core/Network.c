#include "Network.h"
#include "netcode.h"
#include "main.h"
#include "Thread.h"
#include "Log.h"
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include "main.h"
#include "ANSIColourCodes.h"

enum GameRole gRole;

#define TEST_PROTOCOL_ID 0x1122334455667788
#define GAME_PROTOCOL_ID TEST_PROTOCOL_ID
#define CONNECT_TOKEN_EXPIRY 30
#define CONNECT_TOKEN_TIMEOUT 5

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
    Log_Info(buf);
    va_end(args);
}

DECLARE_THREAD_PROC(ClientThread, arg)
{
    netcode_set_printf_function(&NetcodeLog);
    if ( netcode_init() != NETCODE_OK )
    {
        Log_Error( "error: failed to initialize netcode\n" );
        return (void*)1;
    }

    netcode_log_level( NETCODE_LOG_LEVEL_INFO );

    double time = 0.0;
    double delta_time = 1.0 / 60.0;

    Log_Info( "[client]\n" );

    struct netcode_client_config_t client_config;
    netcode_default_client_config( &client_config );
    struct netcode_client_t * client = netcode_client_create( "0.0.0.0", &client_config, time );

    if ( !client )
    {
        Log_Error( "error: failed to create client\n" );
        return (void*)1;
    }

    NETCODE_CONST char* server_address = gCmdArgs.serverAddress;

    uint64_t client_id = 0;
    netcode_random_bytes( (uint8_t*) &client_id, 8 );
    Log_Info( "client id is %.16" PRIx64 "\n", client_id );

    uint8_t user_data[NETCODE_USER_DATA_BYTES];
    netcode_random_bytes(user_data, NETCODE_USER_DATA_BYTES);

    uint8_t connect_token[NETCODE_CONNECT_TOKEN_BYTES];

    if ( netcode_generate_connect_token( 1, &server_address, &server_address, CONNECT_TOKEN_EXPIRY, CONNECT_TOKEN_TIMEOUT, client_id, GAME_PROTOCOL_ID, private_key, user_data, connect_token ) != NETCODE_OK )
    {
        Log_Error( "error: failed to generate connect token\n" );
        return (void*)1;
    }

    netcode_client_connect( client, connect_token );

    bool quit = false;

    while ( !quit )
    {
        netcode_client_update( client, time );

        // if ( netcode_client_state( client ) == NETCODE_CLIENT_STATE_CONNECTED )
        // {
        //     netcode_client_send_packet( client, packet_data, NETCODE_MAX_PACKET_SIZE );
        // }

        // while ( 1 )             
        // {
        //     int packet_bytes;
        //     uint64_t packet_sequence;
        //     void * packet = netcode_client_receive_packet( client, &packet_bytes, &packet_sequence );
        //     if ( !packet )
        //         break;
        //     (void) packet_sequence;
        //     assert( packet_bytes == NETCODE_MAX_PACKET_SIZE );
        //     assert( memcmp( packet, packet_data, NETCODE_MAX_PACKET_SIZE ) == 0 );            
        //     netcode_client_free_packet( client, packet );
        // }

        // if ( netcode_client_state( client ) <= NETCODE_CLIENT_STATE_DISCONNECTED )
        //     break;

        netcode_sleep( delta_time );

        time += delta_time;
    }

    if ( quit )
    {
        Log_Info( "shutting netcode thread down\n" );
    }

    netcode_client_destroy( client );

    netcode_term();
    return NULL;
}

DECLARE_THREAD_PROC(ClientServerThread, arg)
{
    netcode_set_printf_function(&NetcodeLog);
    if ( netcode_init() != NETCODE_OK )
    {
        Log_Error( "failed to initialize netcode\n" );
        return (void*)1;
    }

    netcode_log_level( NETCODE_LOG_LEVEL_INFO );

    double time = 0.0;
    double delta_time = 1.0 / 60.0;

    Log_Info( "[server]\n" );

    NETCODE_CONST char* server_address = gCmdArgs.serverAddress;
    struct netcode_server_config_t server_config;
    netcode_default_server_config( &server_config );
    server_config.protocol_id = GAME_PROTOCOL_ID;
    memcpy( &server_config.private_key, private_key, NETCODE_KEY_BYTES );

    struct netcode_server_t * server = netcode_server_create( server_address, &server_config, time );

    if ( !server )
    {
        Log_Info( "error: failed to create server\n" );
        return (void*)1;
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
        gNetworkThread = StartThread(&ClientThread, NULL);
        break;
    case GR_ClientServer:
        gNetworkThread = StartThread(&ClientServerThread, NULL);
        break;
    }
}

