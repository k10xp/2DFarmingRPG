#include "WfPlayer.h"
#include "BinarySerializer.h"
#include "Game2DLayer.h"
#include "Entities.h"
#include "ObjectPool.h"
#include "DrawContext.h"
#include "GameFramework.h"
#include "AnimatedSprite.h"
#include "Camera2D.h"
#include "WfEntities.h"
#include "WfUI.h"
#include "WfEnums.h"
#include "WfItem.h"
#include "AssertLib.h"
#include "WfPersistantGameData.h"
#include "GameFrameworkEvent.h"
#include "Scripting.h"
#include <string.h>

#define WALKING_UP_MALE "walk-base-male-up"
#define WALKING_DOWN_MALE "walk-base-male-down"
#define WALKING_LEFT_MALE "walk-base-male-left"
#define WALKING_RIGHT_MALE "walk-base-male-right"

#define WALKING_UP_FEMALE "walk-base-female-up"
#define WALKING_DOWN_FEMALE "walk-base-female-down"
#define WALKING_LEFT_FEMALE "walk-base-female-left"
#define WALKING_RIGHT_FEMALE "walk-base-female-right"

#define PLAYER_SPRITE_COMP_INDEX 1
#define PLAYER_COLLIDER_COMP_INDEX 0


static OBJECT_POOL(struct WfPlayerEntData) gPlayerEntDataPool = NULL;


void WfInitPlayer()
{
    gPlayerEntDataPool = NEW_OBJECT_POOL(struct WfPlayerEntData, 4);
}

static void OnInitPlayer(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, DrawContext* pDrawCtx, InputContext* pInputCtx)
{
    Entity2DOnInit(pEnt,pLayer, pDrawCtx, pInputCtx);
    struct WfPlayerEntData* pPlayerEntData = &gPlayerEntDataPool[pEnt->user.hData];
    pPlayerEntData->movementVector[0] = 0.0f;
    pPlayerEntData->movementVector[1] = 0.0f;
    pPlayerEntData->moveUpBinding    = In_FindButtonMapping(pInputCtx, "playerMoveUp");
    pPlayerEntData->moveDownBinding  = In_FindButtonMapping(pInputCtx, "playerMoveDown");
    pPlayerEntData->moveLeftBinding  = In_FindButtonMapping(pInputCtx, "playerMoveLeft");
    pPlayerEntData->moveRightBinding = In_FindButtonMapping(pInputCtx, "playerMoveRight");

    pPlayerEntData->nextItemBinding = In_FindButtonMapping(pInputCtx, "nextItem");
    pPlayerEntData->prevItemBinding = In_FindButtonMapping(pInputCtx, "prevItem");

    pPlayerEntData->settingsMenuBinding = In_FindButtonMapping(pInputCtx, "settings");
    memset(&pPlayerEntData->playerControlsMask, 0, sizeof(struct ActiveInputBindingsMask));
    In_ActivateButtonBinding(pPlayerEntData->moveUpBinding, &pPlayerEntData->playerControlsMask);
    In_ActivateButtonBinding(pPlayerEntData->moveDownBinding, &pPlayerEntData->playerControlsMask);
    In_ActivateButtonBinding(pPlayerEntData->moveLeftBinding, &pPlayerEntData->playerControlsMask);
    In_ActivateButtonBinding(pPlayerEntData->moveRightBinding, &pPlayerEntData->playerControlsMask);
    In_ActivateButtonBinding(pPlayerEntData->settingsMenuBinding, &pPlayerEntData->playerControlsMask);
    In_ActivateButtonBinding(pPlayerEntData->nextItemBinding, &pPlayerEntData->playerControlsMask);
    In_ActivateButtonBinding(pPlayerEntData->prevItemBinding, &pPlayerEntData->playerControlsMask);

    In_SetMask(&pPlayerEntData->playerControlsMask, pInputCtx);
    pPlayerEntData->bMovingThisFrame = false;
    pPlayerEntData->bMovingLastFrame = false;
    pPlayerEntData->bNetworkControlled = false;
    pPlayerEntData->networkPlayerNum = -1;
    pPlayerEntData->metersPerSecondWalkSpeedBase = 100.0f;
    pPlayerEntData->speedMultiplier = 3.0f;
    ClampCameraToTileLayer(pLayer->userData, 0);
}

static void OnDestroyPlayer(struct Entity2D* pEnt, struct GameFrameworkLayer* pData)
{
    FreeObjectPoolIndex(gPlayerEntDataPool, pEnt->user.hData);
    Entity2DOnDestroy(pEnt, pData);
}

static void SetBasePlayerAnimation(enum WfDirection dir, struct GameFrameworkLayer* pLayer, struct WfPlayerEntData* pPlayerEntData, struct Entity2D* pEnt)
{
    struct AnimatedSprite* pSprite = &pEnt->components[PLAYER_SPRITE_COMP_INDEX].data.spriteAnimator;
    pSprite->bIsAnimating = pPlayerEntData->bMovingThisFrame;
    switch(dir)
    {
    case Up:
        AnimatedSprite_SetAnimation(pLayer, pSprite, WALKING_UP_MALE, false, false);
        pSprite->fps *= pPlayerEntData->speedMultiplier;
        break;
    case Down:
        AnimatedSprite_SetAnimation(pLayer, pSprite, WALKING_DOWN_MALE, false, false);
        pSprite->fps *= pPlayerEntData->speedMultiplier;
        break;
    case Left:
        AnimatedSprite_SetAnimation(pLayer, pSprite, WALKING_LEFT_MALE, false, false);
        pSprite->fps *= pPlayerEntData->speedMultiplier;
        break;
    case Right:
        AnimatedSprite_SetAnimation(pLayer, pSprite, WALKING_RIGHT_MALE, false, false);
        pSprite->fps *= pPlayerEntData->speedMultiplier;
        break;
    
    }
}

static void SyncAnimA2B(struct AnimatedSprite* pA, struct AnimatedSprite* pB)
{
    pB->bDraw = pA->bDraw;
    pB->bIsAnimating = pA->bIsAnimating;
    pB->bRepeat = pA->bRepeat;
    pB->fps = pA->fps;
    pB->numSprites = pA->numSprites;
    pB->onSprite = pA->onSprite;
    pB->timer = pA->timer;
}

void WfSetPlayerOverlayAnimations(enum WfDirection dir, struct GameFrameworkLayer* pLayer, struct WfPlayerEntData* pPlayerEntData, struct Entity2D* pEnt)
{
    struct Component2D* pCompBaseAnimator = &pEnt->components[PLAYER_SPRITE_COMP_INDEX];
    struct AnimatedSprite* pBaseAnimatedSprite = &pCompBaseAnimator->data.spriteAnimator;
    EASSERT(pCompBaseAnimator->type == ETE_SpriteAnimator);
    struct GameLayer2DData* pLayerData = pLayer->userData;

    for(int i=0; i<WfNumAnimationLayers; i++)
    {
        if(pPlayerEntData->animationSet.layersMask & (1 << i))
        {
            struct Component2D* pComp = &pEnt->components[PLAYER_SPRITE_COMP_INDEX + 1 + i];
            EASSERT(pComp->type == ETE_SpriteAnimator);
            struct AnimatedSprite* pOverlayAnimator = &pComp->data.spriteAnimator;
            AnimatedSprite_SetAnimation(pLayer, pOverlayAnimator, pPlayerEntData->animationSet.layers[i].animationNames[dir], false, false);
            SyncAnimA2B(pBaseAnimatedSprite, pOverlayAnimator);
        }
    }
}

static void SetPlayerAnimation(struct GameFrameworkLayer* pLayer, struct WfPlayerEntData* pPlayerEntData, struct Entity2D* pEnt)
{
    /* set the base animation */
    struct AnimatedSprite* pSprite = &pEnt->components[PLAYER_SPRITE_COMP_INDEX].data.spriteAnimator;
    pSprite->bIsAnimating = pPlayerEntData->bMovingThisFrame;
    for(int i=0; i<WfNumAnimationLayers; i++)
    {
        if(pPlayerEntData->animationSet.layersMask & (1 << i))
        {
            pEnt->components[PLAYER_SPRITE_COMP_INDEX + 1 + i].data.spriteAnimator.bIsAnimating = pSprite->bIsAnimating;
        }
    }
    if(!pPlayerEntData->bMovingThisFrame && pPlayerEntData->bMovingLastFrame)
    {
        pSprite->onSprite = 0;
        for(int i=0; i<WfNumAnimationLayers; i++)
        {
            if(pPlayerEntData->animationSet.layersMask & (1 << i))
            {
                pEnt->components[PLAYER_SPRITE_COMP_INDEX + 1 + i].data.spriteAnimator.onSprite = 0;
            }
        }
    }
    if(pPlayerEntData->movementVector[1] > 1e-5f)
    {
        // moving down
        SetBasePlayerAnimation(Down, pLayer, pPlayerEntData, pEnt);
        WfSetPlayerOverlayAnimations(Down, pLayer, pPlayerEntData, pEnt);
        pPlayerEntData->directionFacing = Down;
    }
    else if(pPlayerEntData->movementVector[1] < -1e-5f)
    {
        // moving up
        SetBasePlayerAnimation(Up, pLayer, pPlayerEntData, pEnt);
        WfSetPlayerOverlayAnimations(Up, pLayer, pPlayerEntData, pEnt);
        pPlayerEntData->directionFacing = Up;
    }
    else if(pPlayerEntData->movementVector[0] > 1e-5f)
    {
        // moving right
        SetBasePlayerAnimation(Right, pLayer, pPlayerEntData, pEnt);
        WfSetPlayerOverlayAnimations(Right, pLayer, pPlayerEntData, pEnt);
        pPlayerEntData->directionFacing = Right;
    }
    else if(pPlayerEntData->movementVector[0] < -1e-5f)
    {
        // moving left
        SetBasePlayerAnimation(Left, pLayer, pPlayerEntData, pEnt);
        WfSetPlayerOverlayAnimations(Left, pLayer, pPlayerEntData, pEnt);
        pPlayerEntData->directionFacing = Left;
    }
}

static void OnUpdatePlayer(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{
    struct WfPlayerEntData* pPlayerEntData = &gPlayerEntDataPool[pEnt->user.hData];
    switch (pPlayerEntData->state)
    {
    case WfWalking:
        {
            vec2 scaledMovement;
            scaledMovement[0] = pPlayerEntData->movementVector[0] * pPlayerEntData->metersPerSecondWalkSpeedBase * deltaT * pPlayerEntData->speedMultiplier;
            scaledMovement[1] = pPlayerEntData->movementVector[1] * pPlayerEntData->metersPerSecondWalkSpeedBase * deltaT * pPlayerEntData->speedMultiplier;
            Ph_SetDynamicBodyVelocity(
                pEnt->components[PLAYER_COLLIDER_COMP_INDEX].data.dynamicCollider.id,
                scaledMovement
            );
            SetPlayerAnimation(pLayer, pPlayerEntData, pEnt);
        }
        
        break;
    case WfAttacking:
        {
            struct AnimatedSprite* pSprite = &pEnt->components[PLAYER_SPRITE_COMP_INDEX].data.spriteAnimator;
            if(!pSprite->bIsAnimating)
            {
                pPlayerEntData->state = WfWalking;
                pPlayerEntData->bMovingThisFrame = true;
                SetBasePlayerAnimation(pPlayerEntData->directionFacing, pLayer, pPlayerEntData, pEnt);
                WfSetPlayerOverlayAnimations(pPlayerEntData->directionFacing, pLayer, pPlayerEntData, pEnt);
            }
        }
        break;
    default:
        break;
    }
    
    Entity2DUpdate(pEnt, pLayer, deltaT);
}


static struct WfInventory* WfGetPlayerInventory(struct WfPlayerEntData* pEntData)
{
    if(pEntData->bNetworkControlled)
    {
        return WfGetNetworkPlayersInventory(pEntData->networkPlayerNum);
    }
    return WfGetInventory();
}


static void ChangeItem(struct GameFrameworkLayer* pLayer, struct Entity2D* pEnt, struct WfPlayerEntData* pPlayerEntData, int incr)
{
    struct WfInventory* pInv = WfGetPlayerInventory(pPlayerEntData);

    int oldIndex = pInv->selectedItem;
    pInv->selectedItem += incr;
    if(pInv->selectedItem >= WF_INVENTORY_ROW_SIZE)
    {
        pInv->selectedItem = 0;
    }
    if(pInv->selectedItem < 0)
    {
        pInv->selectedItem = WF_INVENTORY_ROW_SIZE - 1;
    }
    struct ScriptCallArgument arg;
    arg.type = SCA_int;
    arg.val.i = pInv->selectedItem;
    struct LuaListenedEventArgs args = { .numArgs = 1, .args = &arg };
    Ev_FireEvent("SelectedItemChanged", &args);
    struct WfItemDef* pOldItem = WfGetItemDef(pInv->pItems[oldIndex].itemIndex);
    struct WfItemDef* pNewItem = WfGetItemDef(pInv->pItems[pInv->selectedItem].itemIndex);
    if(pOldItem)
    {
        pOldItem->onStopBeingCurrent(pEnt, pLayer);
    }
    if(pNewItem)
    {
        pNewItem->onMakeCurrent(pEnt, pLayer);
    }
}

static void OnInputPlayer(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, InputContext* context)
{
    Entity2DInput(pEnt, pLayer, context);
    struct WfPlayerEntData* pPlayerEntData = &gPlayerEntDataPool[pEnt->user.hData];
    pPlayerEntData->movementVector[0] = 0.0f;
    pPlayerEntData->movementVector[1] = 0.0f;
    pPlayerEntData->bMovingLastFrame = pPlayerEntData->bMovingThisFrame;
    pPlayerEntData->bMovingThisFrame = false;
    struct GameLayer2DData* pLayerData = pLayer->userData;
    
    if(In_GetButtonValue(context, pPlayerEntData->moveUpBinding))
    {
        vec2 add = {0.0f, -1.0f};
        glm_vec2_add(pPlayerEntData->movementVector, add, pPlayerEntData->movementVector);
        pPlayerEntData->bMovingThisFrame = true;
    }
    if(In_GetButtonValue(context, pPlayerEntData->moveDownBinding))
    {
        vec2 add = {0.0f, 1.0f};
        glm_vec2_add(pPlayerEntData->movementVector, add, pPlayerEntData->movementVector);   
        pPlayerEntData->bMovingThisFrame = true;
    }
    if(In_GetButtonValue(context, pPlayerEntData->moveLeftBinding))
    {
        vec2 add = {-1.0f, 0.0f};
        glm_vec2_add(pPlayerEntData->movementVector, add, pPlayerEntData->movementVector);   
        pPlayerEntData->bMovingThisFrame = true;
    }
    if(In_GetButtonValue(context, pPlayerEntData->moveRightBinding))
    {
        vec2 add = {1.0f, 0.0f};
        glm_vec2_add(pPlayerEntData->movementVector, add, pPlayerEntData->movementVector);   
        pPlayerEntData->bMovingThisFrame = true;
    }
    if(In_GetButtonPressThisFrame(context, pPlayerEntData->settingsMenuBinding))
    {
        GF_PopGameFrameworkLayer();
        WfPushSettings(pLayerData->pDrawContext);
    }
    if(In_GetButtonPressThisFrame(context, pPlayerEntData->nextItemBinding))
    {
        ChangeItem(pLayer, pEnt, pPlayerEntData, 1);
    }
    if(In_GetButtonPressThisFrame(context, pPlayerEntData->prevItemBinding))
    {
        ChangeItem(pLayer, pEnt, pPlayerEntData, -1);
    }

    glm_vec2_normalize(pPlayerEntData->movementVector);
}

void WfDeSerializePlayerEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
}

void WfSerializePlayerEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{
}

float WfGetPlayerSortPosition(struct Entity2D* pEnt)
{
    return pEnt->transform.position[1] - gPlayerEntDataPool[pEnt->user.hData].groundColliderCenter2EntTransform[1];
}

void WfPlayerGetGroundContactPoint(struct Entity2D* pEnt, vec2 outPos)
{
    struct WfPlayerEntData* pPlayerEntData = &gPlayerEntDataPool[pEnt->user.hData];
    vec2 pos2ground = {
        -pPlayerEntData->groundColliderCenter2EntTransform[0],
        -pPlayerEntData->groundColliderCenter2EntTransform[1],
    };
    glm_vec2_add(pEnt->transform.position, pos2ground, outPos);
}

void WfPlayerPostPhys(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{
    struct WfPlayerEntData* pPlayerEntData = &gPlayerEntDataPool[pEnt->user.hData];
    struct GameLayer2DData* pLayerData = pLayer->userData;
    Entity2DUpdatePostPhysics(pEnt, pLayer, deltaT);
    vec2 pixelsPos, physPos;
    struct DynamicCollider* pCollider = &pEnt->components[PLAYER_COLLIDER_COMP_INDEX].data.dynamicCollider;
    
    Ph_GetDynamicBodyPosition(pCollider->id, physPos);
    Ph_PhysicsCoords2PixelCoords(pLayerData->hPhysicsWorld, physPos, pixelsPos);
    glm_vec2_add(pixelsPos, pPlayerEntData->groundColliderCenter2EntTransform, pEnt->transform.position);

    if(!pPlayerEntData->bNetworkControlled)
    {
        CenterCameraAt(pixelsPos[0], pixelsPos[1], &pLayerData->camera, pLayerData->windowW, pLayerData->windowH);
    }
}

void WfMakeIntoPlayerEntityBase(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, vec2 spawnAtGroundPos, bool bNetworkControlled, int networkPlayerNum)
{
    struct GameLayer2DData* pData = pLayer->userData;
    pEnt->nextSibling = NULL_HANDLE;
    pEnt->previousSibling = NULL_HANDLE;
    gPlayerEntDataPool = GetObjectPoolIndex(gPlayerEntDataPool, &pEnt->user.hData);
    struct WfPlayerEntData* pEntData = &gPlayerEntDataPool[pEnt->user.hData];
    pEntData->directionFacing = Down;
    pEntData->groundColliderCenter2EntTransform[0] = -32;
    pEntData->groundColliderCenter2EntTransform[1] = -60;
    pEntData->animationSet.layersMask = 0;
    pEntData->state = WfWalking;
    pEntData->bNetworkControlled = bNetworkControlled;
    pEntData->networkPlayerNum = networkPlayerNum;

    pEnt->numComponents = 0;
    pEnt->type = WfEntityType_Player;

    /*
        Ground Collider
    */
    struct Component2D* pComponent2 = &pEnt->components[pEnt->numComponents++];
    pComponent2->type = ETE_DynamicCollider;
    pComponent2->data.dynamicCollider.shape.type = PBT_Circle;
    pComponent2->data.dynamicCollider.shape.data.circle.center[0] = spawnAtGroundPos[0];
    pComponent2->data.dynamicCollider.shape.data.circle.center[1] = spawnAtGroundPos[1];
    pComponent2->data.dynamicCollider.shape.data.circle.radius = 10;
    pComponent2->data.dynamicCollider.bIsSensor = false;
    pComponent2->data.dynamicCollider.bGenerateSensorEvents = true;
    pComponent2->data.dynamicCollider.onSensorOverlapBegin = NULL;
    pComponent2->data.dynamicCollider.onSensorOverlapEnd = NULL;

    /*
        Base Animated Sprite
    */
    struct Component2D* pComponent1 = &pEnt->components[pEnt->numComponents++];
    pComponent1->type = ETE_SpriteAnimator;
    struct AnimatedSprite* pAnimatedSprite = &pComponent1->data.spriteAnimator;
    memset(pAnimatedSprite, 0, sizeof(struct AnimatedSprite));
    pAnimatedSprite->animationName = WALKING_DOWN_MALE;
    pAnimatedSprite->bRepeat = true;
    pAnimatedSprite->transform.scale[0] = 1.0f;
    pAnimatedSprite->transform.scale[1] = 1.0f;
    pAnimatedSprite->bIsAnimating = false;
    pAnimatedSprite->bDraw = true;
    

    glm_vec2_add(spawnAtGroundPos, gPlayerEntDataPool[pEnt->user.hData].groundColliderCenter2EntTransform, pEnt->transform.position);
    pEnt->transform.scale[0] = 1.0;
    pEnt->transform.scale[1] = 1.0;

    /* Animated sprite overlay layers */
    for(int i=0; i<WfNumAnimationLayers; i++)
    {
        struct Component2D* pComponent1 = &pEnt->components[pEnt->numComponents++];
        pComponent1->type = ETE_SpriteAnimator;
        struct AnimatedSprite* pAnimatedSprite = &pComponent1->data.spriteAnimator;
        memset(pAnimatedSprite, 0, sizeof(struct AnimatedSprite));
        pAnimatedSprite->animationName = "";
        pAnimatedSprite->bRepeat = true;
        pAnimatedSprite->transform.scale[0] = 1.0f;
        pAnimatedSprite->transform.scale[1] = 1.0f;
        pAnimatedSprite->bIsAnimating = false;
        pAnimatedSprite->bDraw = false;
    }
    

    pEnt->inDrawLayer = 0;

    Et2D_PopulateCommonHandlers(pEnt);
    pEnt->init = &OnInitPlayer;
    pEnt->update = &OnUpdatePlayer;
    pEnt->getSortPos = &WfGetPlayerSortPosition;
    pEnt->postPhys = &WfPlayerPostPhys;
    pEnt->input = &OnInputPlayer;
    pEnt->bKeepInQuadtree = false;
    pEnt->bKeepInDynamicList = true;
    pEnt->bSerializeToDisk = false;
    pEnt->bSerializeToNetwork = true;

    struct WfInventory* pInv = WfGetPlayerInventory(pEntData);
    struct WfInventoryItem* pItem = &pInv->pItems[pInv->selectedItem];
    if(pItem->itemIndex >= 0)
    {
        struct WfItemDef* pDef = WfGetItemDef(pItem->itemIndex);
        pDef->onMakeCurrent(pEnt, pLayer);
    }
}

void WfMakeIntoPlayerEntity(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, vec2 spawnAtGroundPos)
{
    WfMakeIntoPlayerEntityBase(pEnt, pLayer, spawnAtGroundPos, false, -1);
}

struct WfAnimationSet* WfGetPlayerAnimationSet(struct Entity2D* pInPlayerEnt)
{
    struct WfPlayerEntData* pEntData = &gPlayerEntDataPool[pInPlayerEnt->user.hData];
    return &pEntData->animationSet;
}

void WfSetPlayerAnimationSet(struct Entity2D* pInPlayerEnt, const struct WfAnimationSet* pInSet)
{
    struct WfPlayerEntData* pEntData = &gPlayerEntDataPool[pInPlayerEnt->user.hData];
    memcpy(&pEntData->animationSet, pInSet, sizeof(struct WfAnimationSet));
}

struct WfPlayerEntData* WfGetPlayerEntData(struct Entity2D* pInEnt)
{
    return &gPlayerEntDataPool[pInEnt->user.hData];
}


struct Component2D* WfGetPlayerAnimationLayerComponent(struct Entity2D* pPlayer, enum WfAnimationLayerNames layer)
{
    return &pPlayer->components[PLAYER_SPRITE_COMP_INDEX + 1 + (int)layer];
}
