#ifndef GAME2DLAYER_NETWORK_H
#define GAME2DLAYER_NETWORK_H


struct GameFrameworkLayer;
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

void UpdateGame2DLayerNetwork(struct GameFrameworkLayer* pLayer);

//void LoadLevelDataFromSe

#endif
