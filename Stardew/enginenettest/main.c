#include "main.h"
#include "Log.h"
#include "Network.h"
#include "FileHelpers.h"
#include "AssertLib.h"
#include "SharedPtr.h"
#include "cwalk.h"
#include <string.h>
#include <stdio.h>
/*
    extends the engines cmd line args.
    An end to end test written in python will spawn a server process and then a series of short lived clients.
    For each client the script will generate a packet file of random bytes known to the test script and the client process will send it 
    to the server, which save a copy to disk and send another copy (possibly altered) back to the client, which will save that to disk.
    
    The script then compares that saved copies sent to and from the server are the same in contents to the one it generated.

    So this C program needs to :
        if client:
            - accept a file as a command line argument, load it into memory and send it to the server as a reliable packet
             once connected (monitored through connection queue). The test script will enforce a timeout. 
            - listen for a response from the server and save to a file specified on the command line
        if server:
            - listen for an incoming packet from connected clients. It will save these to a file path specified by cmd arg appended with the client number
            - send the packet back to the client
*/



struct NetTestCmdArgs
{
    char* inputFilePath;
    char* outputFilePath;
};

enum GameRole gRole;

static struct  NetTestCmdArgs gTestArgs = {
    .inputFilePath = NULL,
    .outputFilePath = NULL
};

void HandleNetTestArgs(int argc, char** argv, int onArg)
{
    char* arg = argv[onArg];
    
    if(strcmp(arg, "--input_file_path") == 0)
    {
        /* 
            client:
            file containing data to send to the server
            server:
            no effect for server 
        */
        char* file = argv[onArg+1];
        gTestArgs.inputFilePath = file;
    }
    else if(strcmp(arg, "--output_file_path") == 0)
    {
        /* 
            client:
            file that client will dump the servers response to.  
            server:
            file path to dump recieved packet to, will be appended with client index
        */
        char* file = argv[onArg+1];
        gTestArgs.outputFilePath = file;
    }
}

static char* GetFileOutputPath(int clientID)
{
    static char sPathBuf[256];
    char extensionBuf[16];
    strcpy(sPathBuf, gTestArgs.outputFilePath);
    if(cwk_path_has_extension(sPathBuf))
    {
        char* extension = NULL;
        size_t extensionlen = 0;
        cwk_path_get_extension(sPathBuf, (const char**)&extension, &extensionlen);
        /* save the extension for later */
        memset(extensionBuf, 0, 16);
        memcpy(extensionBuf, extension, extensionlen);
        /* remove extension from path */
        memset(extension, 0, extensionlen);
        /* add client id and then extension */
        char* end = sPathBuf + strlen(sPathBuf);
        sprintf(end, "%i%s", clientID, extension);
        Log_Info("Server is saving output to %s", sPathBuf);
    }
    else
    {
        char* end = sPathBuf + strlen(sPathBuf);
        sprintf(end, "%i", clientID);
    }
    return sPathBuf;
}

static int NetTestServer()
{
    while(true)
    {
        struct NetworkQueueItem nci;
        /* 1.) recieve packet */
        while(NW_DequeueData(&nci))
        {
            
            /* 2.) dump to file */
            FILE* pFile = fopen(GetFileOutputPath(nci.client), "w");
            fwrite(nci.pData, 1, nci.pDataSize, pFile);
            fclose(pFile);

            struct NetworkQueueItem response = 
            {
                .bReliable = true,
                .client = nci.client,
                .pData = Sptr_New(nci.pDataSize, NULL),
                .pDataSize = nci.pDataSize
            };

            /* 3.) echo back to client */
            memcpy(response.pData, nci.pData, nci.pDataSize);
            NW_EnqueueData(&response);
        }
    }
}


#define get_tae goto
static int NetTestClient()
{
    int size = 0;
    char* fileContents = LoadFile(gTestArgs.inputFilePath, &size);
    EASSERT(fileContents && size > 0);
    struct NetworkConnectionEvent nce;

    /* wait to connect */
    while(true)
        while(NW_DequeueConnectionEvent(&nce))
        {
            if(nce.type == NCE_ClientConnected)
            {
                get_tae connected;
            }    
        }
connected:

    /* now connected */
    Log_Info("Client: I've connected!");
    struct NetworkQueueItem item = 
    {
        .bReliable = true,
        .client = -1,
        .pData = Sptr_New(size, NULL),
        .pDataSize = size
    };
    memcpy(item.pData, fileContents, size);
    NW_EnqueueData(&item);
    Log_Info("Client: I've sent my packet to the server");
    
    while(true)
        while(NW_DequeueData(&item))
        {
            FILE* pFile = fopen(gTestArgs.outputFilePath, "w");
            fwrite(item.pData, 1, item.pDataSize, pFile);
            fclose(pFile);
            /* don't like gotos? then you can... */
            get_tae fuck;
        }
fuck:

    return 0;
}

int main(int argc, char** argv)
{
    https://www.youtube.com/watch?v=TtYWcA6NnRU

    Engine_ParseCmdArgs(argc, argv, &HandleNetTestArgs);
    Log_Init();
    NW_Init();
    gRole = gCmdArgs.role;
    switch (gCmdArgs.role)
    {
    case GR_Client:
        return NetTestClient();
        break;
    case GR_ClientServer:
        return NetTestServer();
        break;
    default:
        Log_Error("[Net Test] INVALID ROLE %i", gCmdArgs.role);
        break;
    }
    return 0;
}
