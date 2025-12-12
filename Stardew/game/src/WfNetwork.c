#include "WfNetwork.h"
#include "Game2DLayer.h"
#include "Game2DLayerNetwork.h"
#include "BinarySerializer.h"

/*
    Going forwards from the point of initial data exchange with the server a continual exchange will take place.
    - Each client is trusted by the server with regards to the state of its player entity to ensure a responsive feeling game
    - the server continually relays to the player the state of all other players (including its own)
    - both these exchanges will take place via the G2DPacket_WorldState type packet. 
    - A generic handler for G2DPacket_WorldState packets will update entities from the serialized data based on net ids, if it doesn't find the entity with that ID
      it'll create it on the server, if the suggested net id is taken as far as the server is concerned it will send a packet back to the client requiring it to be changed.
      It's likely the one the client has suggested will be correct but a race condition could emerge if two clients spawn entities at the same time, so the server must arbitrate the net IDS.
      The client doesn't have to wait for its netID to be verified, it can continue on so long as it later updates the ID.
    - entity serialization functions can be tuned to pack data tighter if the context is serializing to network
*/

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