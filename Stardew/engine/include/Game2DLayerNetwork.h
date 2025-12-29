
/**
    @file Game2DLayerNetwork.h
    @brief
    Networking layer specific to game 2d layer 
    
*/
#ifndef GAME2DLAYER_NETWORK_H
#define GAME2DLAYER_NETWORK_H

#include "IntTypes.h"
struct GameFrameworkLayer;
struct BinarySerializer;
struct GameLayer2DData;
struct Game2DLayerData;
struct Entity2D;


enum G2DPacketType
{
    G2DPacket_RequestLevelData,
    G2DPacket_LevelDataResponseData,
    G2DPacket_RPC,
    G2DPacket_WorldState
};


struct TilemapEdit
{
    u16 x, y;
    u16 layer;
    TileIndex newVal;
};

enum Game2DRPCType
{
	G2DRPC_CreateEntity,
	G2DRPC_DestroyEntity,
    G2DRPC_AdjustNetworkID,
    G2DRPC_LastEngineRPC /* leave at end */
};

/// @brief callback to construct an rpc message of a given type, pRPCStruct is a struct containing relevant data to construct your rpc message
typedef void(*RPCMessageConstructorFn)(struct BinarySerializer* pBS, void* pRPCStruct);

/// @brief callback to handle an RPC message, to be read from the binary serializer, passed Game2DLayerData* so the RPC can do stuff
typedef void(*RPCMessageHandlerFn)(struct BinarySerializer* pBS, struct GameLayer2DData* pData);

/// @brief An RPC implemented by a users game
struct UserGame2DRPC
{
    /// @brief RPC type - implement an RPC enum type for your game with values starting at G2DRPC_LastEngineRPC, register RPCs in order
    int rpcType; 

    /// @brief callback to construct an rpc message of a given type, pRPCStruct is a struct containing relevant data to construct your rpc message
    RPCMessageConstructorFn ctor;

    /// @brief callback to handle an RPC message, to be read from the binary serializer, passed Game2DLayerData* so the RPC can do stuff
    RPCMessageHandlerFn handler;
};

/// @brief Register a user RPC, call once at startup, call in order your games rpc types appear in the enum
/// @param rpc 
void G2D_RegisterUserRPC(struct UserGame2DRPC* rpc);

typedef void(*PacketExtensionNoArgsFn)(struct BinarySerializer*);

void G2D_Extend_RequestLevelDataMessage(PacketExtensionNoArgsFn fn);

void G2D_Enqueue_RequestLevelData();

enum G2DPacketType G2D_ParsePacket(u8* pPacket, u8** pOutBody, int* outHeaderSize);

/// @brief Enqueue an update consisting of a list of tiles that have changed, a list of entities that have changed
/// @param pData The Game2dLayer
/// @param clientI The client index to send to - only relevant if you're the server
void G2D_Enqueue_Worldstate_Packet(struct GameLayer2DData* pData, int clientI);


/// @brief 
/// @param packet
/// @param pOutBody
enum Game2DRPCType G2D_ParseRPCPacket(u8* packet, u8** pOutBody);

/// @brief do RPCs, call once a packet has been confirmed as a G2DPacket_RPC, call on both client and server
/// @param pData 
/// @param pRPCData 
/// @param client the number of the client who has sent the RPC, or -1 if the server has sent it and you are a client
void G2D_DoRPC(struct GameLayer2DData* pData, u8* pRPCData, int client);

void G2D_PollNetworkQueueClient(struct GameFrameworkLayer* pLayer, float deltaT);

void G2D_PollNetworkQueueServer(struct GameFrameworkLayer* pLayer, float deltaT);

/////////////////////////////////////////////////////////////////////////////////////////////////// RPC data structs

struct AdjustNetID_RPC
{
    int oldNetID;
    int newNetID;
};

struct CreateEntity_RPC
{
    struct Entity2D* pEnt;
    struct Game2DLayerData* pData;
};

struct DeleteEntity_RPC
{
    int netID;
};


#endif
