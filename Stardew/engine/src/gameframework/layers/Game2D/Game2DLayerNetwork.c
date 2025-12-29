#include <string.h>
#include "Game2DLayerNetwork.h"
#include "Log.h"
#include "Network.h"
#include "BinarySerializer.h"
#include "Network.h"
#include "AssertLib.h"
#include "DynArray.h"
#include "Entities.h"
#include "Game2DLayer.h"
#include "NetworkID.h"
#include "GameFramework.h"
#include <stdbool.h>


struct GameThreadClientConnection
{
    bool connected;
};

static struct GameThreadClientConnection gClientsConnections[3] = { {.connected=false}, {.connected=false}, {.connected=false} };
static struct NetworkQueueItem gDequeueBuffer[128];
static PacketExtensionNoArgsFn gExtendRequestLvlData = NULL;
static VECTOR(struct UserGame2DRPC) gUserRPCs = NULL;


void G2D_RegisterUserRPC(struct UserGame2DRPC* rpc)
{
	if(!gUserRPCs)
	{
		gUserRPCs = NEW_VECTOR(struct UserGame2DRPC);
	}
	EASSERT(rpc->rpcType - G2DRPC_LastEngineRPC == VectorSize(gUserRPCs)); /* You need to register the RPCs in the right order, the order they appear in your games enum */
	gUserRPCs = VectorPush(gUserRPCs, rpc);
}

static struct NetworkQueueItem* GetLatestStateUpdate(VECTOR(struct NetworkQueueItem) dequeuedStateUpdates)
{
	struct NetworkQueueItem* latest = NULL;
	if(VectorSize(dequeuedStateUpdates))
	{
		latest = &dequeuedStateUpdates[0];
		/* we're only interested in the latest one.  */
		for(int i=0; i<VectorSize(dequeuedStateUpdates); i++)
		{
			if(dequeuedStateUpdates[i].sequenceNumber > latest->sequenceNumber)
			{
				latest = &dequeuedStateUpdates[i];
			}
		}	
	}
	return latest;
}



static void ApplyStateUpdate(struct GameLayer2DData* pData)
{

}

void G2D_PollNetworkQueueServer(struct GameFrameworkLayer* pLayer, float deltaT)
{
	struct GameLayer2DData* pData = pLayer->userData;

	struct NetworkConnectionEvent nce;
	while(NW_DequeueConnectionEvent(&nce))
	{
		switch(nce.type)
		{
		case NCE_ClientConnected:
			Log_Info("Client %i connected", nce.client);
            gClientsConnections[nce.client].connected = true;
			break;
		case NCE_ClientDisconnected:
			Log_Info("Client %i disconnected", nce.client);
            gClientsConnections[nce.client].connected = false;
			break;
		}
	}

	static VECTOR(struct NetworkQueueItem) dequeuedStateUpdates = NULL;
	if(!dequeuedStateUpdates)
	{
		dequeuedStateUpdates = NEW_VECTOR(struct NetworkQueueItem);
	}

	struct NetworkQueueItem nqi;
	while(NW_DequeueData(&nqi))
	{
		Log_Info("Recieved data from client %i", nqi.client);
		u8* pBody = NULL;
		int headerSize = 0;
		enum G2DPacketType type = G2D_ParsePacket(nqi.pData, &pBody, &headerSize);
		switch(type)
		{
		case G2DPacket_RequestLevelData:
			{
				Log_Info("recieved level data request from client %i,", nqi.client);
				/* handle request */
				if(pData->levelDataRequestHandlerExtender)
				{
					struct BinarySerializer bs;
					BS_CreateForLoadFromBuffer(pBody, nqi.pDataSize - headerSize, &bs);
					pData->levelDataRequestHandlerExtender(pData, &bs);
					BS_Finish(&bs); /* does nothing if BS_CreateForLoadFromBuffer but added anyway */
				}
				Log_Info("sending level data response to client %i,", nqi.client);
				/* write response */
				struct BinarySerializer bs;
				BS_CreateForSaveToNetwork(&bs, nqi.client);
				BS_SerializeU32(G2DPacket_LevelDataResponseData, &bs);
				
				if(pData->levelDataPacketExtender)
				{
					pData->levelDataPacketExtender(pData, &bs);
				}

				G2D_SaveLevelDataInternal(pData, &bs);
				BS_Finish(&bs);
			}
			break;
		case G2DPacket_LevelDataResponseData:
			EASSERT(false);
			break;
		case G2DPacket_RPC:
			G2D_DoRPC(pLayer, pLayer->userData, pBody, nqi.client);
			break;
		case G2DPacket_WorldState:
			VectorPush(dequeuedStateUpdates, &nqi);
			break;
		default:
			Log_Error("Game2DLayer server recieved unknown packet type (%i)");
			break;
		}
	}
	struct NetworkQueueItem* pNqi = GetLatestStateUpdate(dequeuedStateUpdates);
	if(pNqi)
	{
		/* apply the latest state update */
		dequeuedStateUpdates = VectorClear(dequeuedStateUpdates);
	}
	
}

void G2D_PollNetworkQueueClient(struct GameFrameworkLayer* pLayer, float deltaT)
{
	struct GameLayer2DData* pData = pLayer->userData;

	struct NetworkConnectionEvent nce;

	static VECTOR(struct NetworkQueueItem) dequeuedStateUpdates = NULL;
	if(!dequeuedStateUpdates)
	{
		dequeuedStateUpdates = NEW_VECTOR(struct NetworkQueueItem);
	}

	while(NW_DequeueConnectionEvent(&nce))
	{
		switch(nce.type)
		{
		case NCE_ClientConnected:
			Log_Info("Client %i connected", nce.client);
			break;
		case NCE_ClientDisconnected:
			Log_Info("Client %i disconnected", nce.client);
			break;
		}
	}

	struct NetworkQueueItem nqi;
	while(NW_DequeueData(&nqi))
	{
		u8* pBody = NULL;
		int headerSize = 0;
		enum G2DPacketType type = G2D_ParsePacket(nqi.pData, &pBody, &headerSize);
		switch(type)
		{
		case G2DPacket_RequestLevelData:
			EASSERT(false);
			break;
		case G2DPacket_LevelDataResponseData:
			EASSERT(false);
			break;
		case G2DPacket_RPC:
			G2D_DoRPC(pLayer, pLayer->userData, pBody, nqi.client);
			break;
		case G2DPacket_WorldState:
			VectorPush(dequeuedStateUpdates, &nqi);
			break;
		}
	}
	struct NetworkQueueItem* pNqi = GetLatestStateUpdate(dequeuedStateUpdates);
	if(pNqi)
	{
		/* apply the latest state update */
		dequeuedStateUpdates = VectorClear(dequeuedStateUpdates);
	}
}


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
}

enum Game2DRPCType G2D_ParseRPCPacket(u8* packet, u8** pOutBody)
{
	enum Game2DRPCType type = *((enum Game2DRPCType*)packet);
	*pOutBody = packet + sizeof(enum Game2DRPCType);
	return type;
}

struct IsNetIdTakenIteratorContext
{
    bool bReturnVal;
    int proposedNetId;
};

bool IsNetIDTakenIterator(struct Entity2D* pEnt, int i, void* pUser)
{
    struct IsNetIdTakenIteratorContext* pCtx = pUser;
    pCtx->bReturnVal = false;
    if(pEnt->networkID == pCtx->proposedNetId)
    {
        pCtx->bReturnVal = true;
        return false;
    }
    return true;
}

static bool G2D_IsNetIDTaken(int netID, struct Entity2DCollection* pCollection)
{
    struct IsNetIdTakenIteratorContext ctx = 
	{
        .bReturnVal = false,
        .proposedNetId = netID
    };
    Et2D_IterateEntities(pCollection, &IsNetIDTakenIterator, &ctx);
    return ctx.bReturnVal;
}

struct FindWithNetIdIteratorContext
{
	int netID;
	struct Entity2D* pOutEnt;
};

static bool FindWithNetIDIterator(struct Entity2D* pEnt, int i, void* pUser)
{
	struct FindWithNetIdIteratorContext* pCTX = pUser;
	if(pEnt->networkID == pCTX->netID)
	{
		pCTX->pOutEnt = pEnt;
		return false;
	}
	return true;
}

static struct Entity2D* G2D_FindEntityWithNetID(struct Entity2DCollection* pCollection, int netID)
{
	struct FindWithNetIdIteratorContext ctx = 
	{
		.netID = netID,
		.pOutEnt = NULL
	};
	Et2D_IterateEntities(pCollection, FindWithNetIDIterator, &ctx);
	return ctx.pOutEnt;
}

void G2D_SendRPC(int client, enum Game2DRPCType type, void* pRPCData)
{
    struct BinarySerializer bs;
    BS_CreateForSaveToNetwork(&bs, client);
    BS_SerializeU32(G2DPacket_RPC, &bs);
    BS_SerializeU32(type, &bs);
    switch(type)
    {
    case G2DRPC_CreateEntity:
        {
            struct CreateEntity_RPC* pData = pRPCData;
            BS_SerializeU32(1, &bs); /* version */
            BS_SerializeI32(pData->pEnt->inDrawLayer, &bs);
            Et2D_SerializeEntityV1Base(pData->pEnt, &bs, pData);
        }
        break;
    case G2DRPC_AdjustNetworkID:
        {
            struct AdjustNetID_RPC* pData = pRPCData;
            BS_SerializeI32(pData->oldNetID, &bs);
            BS_SerializeI32(pData->newNetID, &bs);
        }
        break;
    case G2DRPC_DestroyEntity:
        {
			struct DeleteEntity_RPC* pData = pRPCData;
			BS_SerializeI32(pData->netID, &bs);
        }
        break;
    default:
        if(gUserRPCs)
		{
			if(type - G2DRPC_LastEngineRPC < VectorSize(gUserRPCs))
			{
				gUserRPCs[type].ctor(&bs, pRPCData);
			}
			else
			{
				Log_Error("G2D_SendRPC unknown type out of range of user RPCs: %i", type);
			}
		}
		else
		{
			Log_Error("G2D_SendRPC (no user rpcs registered) unknown type out of range of user RPCs: %i", type);
		}
        break;
    }
    BS_Finish(&bs);
}

void G2D_DoRPC(struct GameFrameworkLayer* pLayer, struct GameLayer2DData* pData, u8* pRPCData, int client)
{
	u8* rpcPacketBody = NULL;
	enum Game2DRPCType type = G2D_ParseRPCPacket(pRPCData, &rpcPacketBody);
	struct BinarySerializer bs;
	BS_CreateForLoadFromBuffer(rpcPacketBody, 
		100000000, /* just an arbitrary large size: todo, use actual size */ 
		&bs
	);
	switch(type)
	{
	case G2DRPC_CreateEntity:
		{
			
            u32 version = 0;
            BS_DeSerializeU32(&version, &bs);
            switch (version)
            {
            case 1:
                {
                    struct Entity2D ent;
                    int layer = 0;
                    memset(&ent, 0, sizeof(struct Entity2D));
                    BS_DeSerializeI32(&layer, &bs);
                    Et2D_DeserializeEntityV1Base(&pData->entities, &bs, pData, layer, &ent);
                    if(client >= 0)
                    {
                        EASSERT(NW_GetRole() == GR_ClientServer);
                        /* check if net ID is taken */
                        if(G2D_IsNetIDTaken(ent.networkID, &pData->entities))
                        {
                            int oldNetID = ent.networkID;
                            ent.networkID = NetID_GetID();
                            /* send an rpc back to client here to make it change its net ID to match this one */
                            struct AdjustNetID_RPC data = 
                            {
                                .newNetID = ent.networkID,
                                .oldNetID = oldNetID
                            };
                            G2D_SendRPC(client, G2DRPC_AdjustNetworkID, &data);
                        }
                        /* relay rpc to other clients */
                        for(int i=0; i<3; i++)
                        {
                            if(i != client && gClientsConnections[i].connected)
                            {
                                /* send RPC here */
                                struct CreateEntity_RPC rpc = 
                                {
                                    .pData = pData,
                                    .pEnt = &ent
                                };
                                G2D_SendRPC(i, G2DRPC_CreateEntity, &rpc);
                            }
                        } 
                    }

					/* TODO: perhaps init-ing entities should take place at a given point in the loop */
                    HEntity2D hEnt = Et2D_AddEntityNoNewNetID(&pData->entities, &ent);
					struct Entity2D* pEnt = Et2D_GetEntity(&pData->entities, hEnt);
					pEnt->init(pEnt, pLayer, pData->pDrawContext, NULL);
                }
                break;
            default:
                Log_Error("Create entity RPC, unknown packet schema version");
                break;
            }
		}
		break;
    case G2DRPC_AdjustNetworkID:
        {
            EASSERT(NW_GetRole() == GR_Client);
			
			i32 oldEntID = -1;
			i32 newEntID = -1;
            BS_DeSerializeI32(&oldEntID, &bs);
			BS_DeSerializeI32(&newEntID, &bs);
			struct Entity2D* pEnt = G2D_FindEntityWithNetID(&pData->entities, oldEntID);
			if(pEnt)
			{
				pEnt->networkID = newEntID;
			}
			else
			{
				Log_Error("G2DRPC_AdjustNetworkID - no entity could be found with network id: %i", oldEntID);
			}
        }
        break;
	case G2DRPC_DestroyEntity:
		{
			i32 ent2DestroyNetID = -1;
			BS_DeSerializeI32(&ent2DestroyNetID, &bs);
			struct Entity2D* pEnt = G2D_FindEntityWithNetID(&pData->entities, ent2DestroyNetID);
			if(pEnt)
			{
				Et2D_DestroyEntity(pData->pLayer, &pData->entities, pEnt->thisEntity);
			}
			else
			{
				Log_Error("G2DRPC_DestroyEntity - no entity could be found with network id: %i", ent2DestroyNetID);
			}
		}
		break;
	default:
		{
			if(gUserRPCs)
			{
				if(type - G2DRPC_LastEngineRPC < VectorSize(gUserRPCs))
				{
					gUserRPCs[type].handler(&bs, pData);
				}
				else
				{
					Log_Error("G2D_DoRPC unknown type out of range of user RPCs: %i", type);
				}
			}
			else
			{
				Log_Error("G2D_DoRPC (no user rpcs registered) unknown type out of range of user RPCs: %i", type);
			}
		}
		break;
	}
	BS_Finish(&bs); /* should do nothing */
}
