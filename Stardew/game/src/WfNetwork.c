#include "WfNetwork.h"
#include "Game2DLayer.h"
#include "Game2DLayerNetwork.h"
#include "BinarySerializer.h"



static void Client_ExtendRequestLevelData(struct BinarySerializer* pBS)
{
    // Serialize the clients persistent data here. Client will select a save file with cmd line args
}

static void Server_HandleExtraRequestLevelData(struct GameLayer2DData* pGameLayerData, struct BinarySerializer* pBS)
{
    // handle the clients persistent data on the server here. It will stash it and then add it after to the end of the networked clients list.
    // When the client has created a player entity for itself it will serialize it and send it to the server and the serialization method below should I think 
    // mean that it has a correct persistent data index as far as the server is concerned.
}

static void Server_LevelDataPacketExtender(struct GameLayer2DData* pGameLayerData, struct BinarySerializer* pBS)
{
    // write my persistent data and that of all other connected clients to be read by the client
    // before they load the main level data, the player entities in which will refer to persistent data
    //
    // The server will serialize the connected clients first, in order, and then at the end its own persistant data,
    // that way when the entities are serialized the indexes into gNetworkPlayersPersistantData will still be correct for the client
    // if the server player entities directly serialize that number.
    // The player entity serialization function will have to serialize what the correct index will be for the client.
    //  - this should just be a case of, "if the networkedPlayerNum == -1, serialize the maximum client index + 1"
}

static void Client_LevelDataHandlerExtender(struct GameLayer2DData* pGameLayerData, struct BinarySerializer* pBS)
{
    // load the persistent data of the other connected players.
    // It will arrive as a flat list of persistant data objects which will be populated into the
    // gNetworkPlayersPersistantData in order.
    // When the player entities are deserialized as part of deserializing the main level data, their networkPlayerNum values 
    // should index into the correct position of gNetworkPlayersPersistantData
}

void WfNetworkInit(struct GameLayer2DData* pLayer)
{
    pLayer->levelDataPacketExtender = &Server_LevelDataPacketExtender;
    pLayer->levelDataHandlerExtender = &Client_LevelDataHandlerExtender;
    pLayer->levelDataRequestHandlerExtender = &Server_HandleExtraRequestLevelData;
    G2D_Extend_RequestLevelDataMessage(&Client_ExtendRequestLevelData);
}