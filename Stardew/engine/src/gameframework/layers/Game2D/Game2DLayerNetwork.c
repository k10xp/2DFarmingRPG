#include "Game2DLayerNetwork.h"
#include "Log.h"
#include "Network.h"
#include "BinarySerializer.h"
#include "Network.h"
#include "AssertLib.h"
#include "DynArray.h"

static struct NetworkQueueItem gDequeueBuffer[128];
static PacketExtensionNoArgsFn gExtendRequestLvlData = NULL;
static VECTOR(struct TilemapEdit) gEdits = NULL;

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

void G2D_InformNetworkOfTilemapEdit(struct TilemapEdit* pEdit)
{
    if(!gEdits)
    {
        gEdits = NEW_VECTOR(struct TilemapEdit);
        gEdits = VectorResize(gEdits, 4);
    }
    VectorPush(gEdits, pEdit);
}

struct Game2DLayerWorldstatePacketHeader
{
    u16 numEntities;
    u16 entityLayer; /* assumes all entities on one layer */
};


void G2D_Enqueue_Worldstate_Packet(struct GameLayer2DData* pData, int clientI)
{
    struct BinarySerializer bs;
    BS_CreateForSaveToNetwork(&bs, clientI);
    BS_SerializeI32(G2DPacket_WorldState, &bs);
    // struct Game2DLayerWorldstatePacketHeader header = {
    //     .numTilemapEdits = VectorSize(gEdits),
    //     .
    // };
}

