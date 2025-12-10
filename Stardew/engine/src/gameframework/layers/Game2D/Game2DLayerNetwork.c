#include "Game2DLayerNetwork.h"
#include "Log.h"
#include "Network.h"
#include "BinarySerializer.h"
#include "Network.h"
#include "AssertLib.h"

static struct NetworkQueueItem gDequeueBuffer[128];
static PacketExtensionNoArgsFn gExtendRequestLvlData = NULL;

void G2D_Extend_RequestLevelDataMessage(PacketExtensionNoArgsFn fn)
{
    gExtendRequestLvlData = fn;
}

void G2D_Enqueue_RequestLevelData()
{
    EASSERT(NW_GetRole() == GR_Client);
    struct BinarySerializer bs;
    BS_CreateForSaveToNetwork(&bs, -1);
    BS_SerializeU32(G2DPacket_RequestLevelData, &bs);
    if(gExtendRequestLvlData)
    {
        gExtendRequestLvlData(&bs);
    }
    BS_Finish(&bs);
}

enum G2DPacketType G2D_ParsePacket(u8* pPacket, u8** pOutBody, int* outHeaderSize)
{
    *pOutBody = pPacket + sizeof(enum G2DPacketType);
    *outHeaderSize = sizeof(enum G2DPacketType);
    return *((enum G2DPacketType*)pPacket);
}
