#ifndef NETWORK_H
#define NETWORK_H

#include "ThreadSafeQueue.h"
#include "SharedPtr.h"
#include <stdlib.h>

/*
    assume, for now:
    - the data is malloc'd
    - client and server, whoever dequeus the packet, frees it 
*/
struct NetworkQueueItem
{
    /*
        if its a server recieve queue this is the sending client
        if its a server transmit queue then this is what client it should be sent to.

        On the client this is meaningless as communicaiton is always to and from the server
    */
    int client;
    SHARED_PTR(void) pData;
    int pDataSize;
    bool bReliable
};

enum NetworkConnectionEventType
{
    NCE_ClientConnected,
    NCE_ClientDisconnected
};

struct NetworkConnectionEvent
{
    enum NetworkConnectionEventType type;
    int client;
};

struct NetworkThreadQueues
{
    /* queue of struct NetworkQueueItem transmit to network */
    struct ThreadSafeQueue tx;
    /* queue of struct NetworkQueueItem recieve from network */
    struct ThreadSafeQueue rx;

    struct ThreadSafeQueue connectionEvents;
};

struct HostInfo
{
    const char* ip;
    unsigned short port;
};

enum GameRole
{
    GR_Singleplayer,
    GR_Client,
    GR_ClientServer
};

void NW_Init();

#endif
