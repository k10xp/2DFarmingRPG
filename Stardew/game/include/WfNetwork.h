#ifndef WFNETWORK_H
#define WFNETWORK_H

#include "IntTypes.h"
#include "Game2DLayerNetwork.h"

struct GameLayer2DData;

struct ItemChangeRPCArgs
{
    u8 newActiveItem;
};

enum WfRPCs
{
    WfPlayerChangeItemRPC = G2DRPC_LastEngineRPC,
};

/// @brief callbacks for Game2DLayer networking
/// @param pLayer 
void WfNetworkInit(struct GameLayer2DData* pLayer);

#endif
