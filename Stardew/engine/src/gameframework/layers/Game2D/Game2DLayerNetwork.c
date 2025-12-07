#include "Game2DLayerNetwork.h"
#include "Log.h"
#include "Network.h"

static struct NetworkQueueItem gDequeueBuffer[128];


void UpdateGame2DLayerNetwork(struct GameFrameworkLayer* pLayer)
{
    int numDequeued=0;
    while(NW_DequeueData(&gDequeueBuffer[numDequeued++]))
    {

    }
    for(int i=0; i<numDequeued; i++)
    {
        struct Game2DLayerPacketHeader* pHeader = gDequeueBuffer[i].pData;
        switch(pHeader->type)
        {
        case G2DPacket_RequestLevelData:
            if(NW_GetRole() == GR_ClientServer)
            {
                
            }
            else
            {
                Log_Error("Recieving G2DPacket_RequestLevelData when the role is not server! something badly wrong");
            }
            break;
        }
    }
}

