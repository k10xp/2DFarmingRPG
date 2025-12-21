#ifndef WFPLAYER_H
#define WFPLAYER_H

#include <cglm/cglm.h>
#include "WfEnums.h"
#include "InputContext.h"


struct BinarySerializer;
struct Entity2D;
struct Component2D;
struct GameLayer2DData;
struct GameFrameworkLayer;

#define NUM_ANIMATIONS NumDirections

struct WfAnimationSetLayer
{
    const char* animationNames[NUM_ANIMATIONS];
};

/* layers on top of the base */
enum WfAnimationLayerNames
{
    WfHairAnimationLayer,
    WfTorsoAnimationLayer,
    WfLegAnimationLayer,
    WfToolAnimationLayer,
    WfNumAnimationLayers
};

/* a set of animations that overlay the base animation in layers */
struct WfAnimationSet
{
    struct WfAnimationSetLayer layers[WfNumAnimationLayers];
    unsigned int layersMask;
};

enum WfPlayerState
{
    WfWalking,
    WfAttacking,
    WfNumPlayerStates
};

struct WfPlayerEntData
{
    vec2 groundColliderCenter2EntTransform;
    struct ButtonBinding moveUpBinding;
    struct ButtonBinding moveDownBinding;
    struct ButtonBinding moveLeftBinding;
    struct ButtonBinding moveRightBinding;

    struct ButtonBinding nextItemBinding;
    struct ButtonBinding prevItemBinding;


    struct ButtonBinding settingsMenuBinding;
    struct ActiveInputBindingsMask playerControlsMask;
    /* value I set this to is NOT meters per second, TODO: fix */
    float metersPerSecondWalkSpeedBase;

    float speedMultiplier;

    vec2 movementVector;

    struct WfAnimationSet animationSet;

    enum WfDirection directionFacing;

    enum WfPlayerState state;

    /* 
        its shit having to have this, the position to spawn the network player when it is initialized.
        The network player is created in two steps, first they're deserialized into a player entity then the player entity is initialized.
        We have to call WfMakeIntoPlayerEntityBase when it is initialized, and so must serialize args for it in the deserialize stage
    */
    struct 
    {
        vec2 netPlayerSpawnAtPos; 
        int netPlayerSlot;
    } createNetPlayerOnInitArgs;
    

    int networkPlayerNum;

    /* flags section */
    u32 bMovingThisFrame : 1;
    u32 bMovingLastFrame : 1;
    u32 bNetworkControlled : 1;
};

void WfInitPlayer();

struct WfAnimationSet* WfGetPlayerAnimationSet(struct Entity2D* pInPlayerEnt);

void WfSetPlayerAnimationSet(struct Entity2D* pInPlayerEnt, const struct WfAnimationSet* pInSet);

void WfDeSerializePlayerEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData);

void WfSerializePlayerEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData);

void WfMakeIntoPlayerEntity(struct Entity2D* pInEnt, struct GameFrameworkLayer* pLayer, vec2 spawnAtGroundPos);

struct WfPlayerEntData* WfGetPlayerEntData(struct Entity2D* pInEnt);

struct Component2D* WfGetPlayerAnimationLayerComponent(struct Entity2D* pPlayer, enum WfAnimationLayerNames layer);

void WfSetPlayerOverlayAnimations(enum WfDirection dir, struct GameFrameworkLayer* pLayer, struct WfPlayerEntData* pPlayerEntData, struct Entity2D* pEnt);

void WfPlayerGetGroundContactPoint(struct Entity2D* pEnt, vec2 outPos);

#endif