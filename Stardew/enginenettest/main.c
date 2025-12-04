#include "main.h"
#include "Log.h"
#include "Network.h"
#include <string.h>
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
};

static struct  NetTestCmdArgs gArgs;


static void NetTestServer()
{
    
}

static void NetTestClient()
{

}

void ArgHandler(int argc, char** argv, int i)
{
    
}

int main(int argc, char** argv)
{
    Engine_ParseCmdArgs(argc, argv, NULL);
    Log_Init();
    NW_Init();
    switch (gCmdArgs.role)
    {
    case GR_Client:
        NetTestClient();
        break;
    case GR_ClientServer:
        NetTestServer();
        break;
    default:
        Log_Error("[Net Test] INVALID ROLE %i", gCmdArgs.role);
        break;
    }
}
