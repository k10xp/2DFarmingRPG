#ifndef GAME2DLAYER_NETWORK_H
#define GAME2DLAYER_NETWORK_H


struct GameFrameworkLayer;
struct BinarySerializer;
/* Networking layer specific to game 2d layer */

enum G2DPacketType
{
    G2DPacket_RequestLevelData,
    G2DPacket_LevelDataResponseData,
    G2DPacket_RPC,
    G2DPacket_WorldState
};

struct Game2DLayerPacketHeader
{
    enum G2DPacketType type;
};

typedef void(*PacketExtensionNoArgsFn)(struct BinarySerializer*);

void G2D_Extend_RequestLevelDataMessage(PacketExtensionNoArgsFn fn);

void G2D_Enqueue_RequestLevelData();

//void LoadLevelDataFromSe

#endif
