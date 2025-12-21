#ifndef GAME2DLAYER_NETWORK_H
#define GAME2DLAYER_NETWORK_H

#include "IntTypes.h"
struct GameFrameworkLayer;
struct BinarySerializer;
struct GameLayer2DData;
/* Networking layer specific to game 2d layer */

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

typedef void(*PacketExtensionNoArgsFn)(struct BinarySerializer*);

void G2D_Extend_RequestLevelDataMessage(PacketExtensionNoArgsFn fn);

void G2D_Enqueue_RequestLevelData();

enum G2DPacketType G2D_ParsePacket(u8* pPacket, u8** pOutBody, int* outHeaderSize);

void G2D_InformNetworkOfTilemapEdit(struct TilemapEdit* pEdit);

/// @brief Enqueue an update consisting of a list of tiles that have changed, a list of entities that have changed
/// @param pData The Game2dLayer
/// @param clientI The client index to send to - only relevant if you're the server
void G2D_Enqueue_Worldstate_Packet(struct GameLayer2DData* pData, int clientI);

#endif
