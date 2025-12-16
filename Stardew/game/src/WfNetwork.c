#include "WfNetwork.h"
#include "Game2DLayer.h"
#include "Game2DLayerNetwork.h"
#include "BinarySerializer.h"
#include "WfPersistantGameData.h"
#include "Network.h"
#include "Log.h"
#include "AssertLib.h"
#include "WfWorld.h"
#include <string.h>

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
struct WfPersistantData;

static void Client_ExtendRequestLevelData(struct BinarySerializer* pBS)
{
    Log_Info("Client_ExtendRequestLevelData. Role: %s", NW_GetRole() == GR_Client ? "client" : "something other than client");
    // Serialize the clients persistent data here. Client will select a save file with cmd line args
    EASSERT(NW_GetRole() == GR_Client);
    BS_SerializeU32(1, pBS); // schema version
    struct WfPersistantData* pPersistantData = WfGetLocalPlayerPersistantGameData();
    WfSavePersistantDataFileInternal(pBS, pPersistantData);
}

/*
    inelegant but should work
*/
static struct WfPersistantData gStashedPersistentData;

static void Server_HandleExtraRequestLevelData(struct GameLayer2DData* pGameLayerData, struct BinarySerializer* pBS)
{
    // handle the clients persistent data on the server here. It will stash it and then add it after to the end of the networked clients list.
    // When the client has created a player entity for itself it will serialize it and send it to the server and the serialization method below should I think 
    // mean that it has a correct persistent data index as far as the server is concerned.
    u32 version = 0;
    BS_DeSerializeU32(&version, pBS);
    switch(version)
    {
    case 1:
        {
            WfLoadPersistantDataFileInternal(pBS, &gStashedPersistentData);
        }
        break;
    default:
        Log_Error("Server_HandleExtraRequestLevelData: Unknown schema version %u", version);
    }
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
    int numNetworkedPlayerSlot = WfGetNumNetworkPlayerPersistentDataSlots();
    Log_Info("Server num slots %i", numNetworkedPlayerSlot);

    BS_SerializeU32(1, pBS); /* message schema version */
    BS_SerializeU32(numNetworkedPlayerSlot + 1, pBS); /* number of slots in total in the message */
    for(int i=0; i<numNetworkedPlayerSlot; i++)
    {
        struct WfPersistantData* pData = WfGetNetworkPlayerPersistantGameData(i);
        WfSavePersistantDataFileInternal(pBS, pData);
    }
    struct WfPersistantData* pData = WfGetLocalPlayerPersistantGameData();
    WfSavePersistantDataFileInternal(pBS, pData);
    BS_SerializeString(WfWorld_GetPreviousLocationName(), pBS);
    /*
        Add the connected client we stashed before - this is stupid, TODO: fix it
    */
    int numSlots = WfGetNumNetworkPlayerPersistentDataSlots();
    int next = numSlots;
    WfSetNumNetworkPlayerPersistentDataSlots(++numSlots);
    pData = WfGetNetworkPlayerPersistantGameData(next);
    memcpy(pData, &gStashedPersistentData, sizeof(struct WfPersistantData));
}

static void Client_LevelDataHandlerExtender(struct GameLayer2DData* pGameLayerData, struct BinarySerializer* pBS)
{
    // load the persistent data of the other connected players.
    // It will arrive as a flat list of persistant data objects which will be populated into the
    // gNetworkPlayersPersistantData in order.
    // When the player entities are deserialized as part of deserializing the main level data, their networkPlayerNum values 
    // should index into the correct position of gNetworkPlayersPersistantData
    u32 version = 0;
    BS_DeSerializeU32(&version, pBS);
    switch(version)
    {
    case 1:
        {
            u32 numPlayers = 0;
            BS_DeSerializeU32(&numPlayers, pBS);
            Log_Info("Number of networked players: %u", numPlayers);
            WfSetNumNetworkPlayerPersistentDataSlots((int)numPlayers);
            for(int i=0; i<numPlayers; i++)
            {
                Log_Info("Loading networked player slot %i", i);
                struct WfPersistantData* pData = WfGetNetworkPlayerPersistantGameData(i);
                WfLoadPersistantDataFileInternal(pBS, pData);
            }
            char currentLocation[MAX_LOCATION_NAME_LEN];
            BS_DeSerializeStringInto(currentLocation, pBS);
            WfWorld_SetCurrentLocationName(currentLocation);
        }
        break;
    default:
        Log_Error("Unknown persistent data schema version from server: %u", version);
        break;
    }
}

void WfNetworkInit(struct GameLayer2DData* pLayer)
{
    /*
        misc
    */
    gStashedPersistentData.inventory.pItems = NEW_VECTOR(struct WfInventoryItem);
    /*
        Callbacks into Game2DLayer networking
        - Initial exchange of level and player data when a client connects
    */
    Log_Info("WfNetworkInit");
    pLayer->levelDataPacketExtender = &Server_LevelDataPacketExtender;
    pLayer->levelDataHandlerExtender = &Client_LevelDataHandlerExtender;
    pLayer->levelDataRequestHandlerExtender = &Server_HandleExtraRequestLevelData;
    G2D_Extend_RequestLevelDataMessage(&Client_ExtendRequestLevelData);
    /*
        - RPCs
    */
}