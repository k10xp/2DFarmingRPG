#include "WfBasicAxe.h"
#include "WfItem.h"
#include "WfPlayer.h"
#include "Entities.h"

#include <stdlib.h>


static void OnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    pEntData->animationSet.layersMask |= (1 << WfToolAnimationLayer);
    pEntData->animationSet.layers[WfToolAnimationLayer].animationNames[Up] = "walk-axe-male-up";
    pEntData->animationSet.layers[WfToolAnimationLayer].animationNames[Down] = "walk-axe-male-down";
    pEntData->animationSet.layers[WfToolAnimationLayer].animationNames[Left] = "walk-axe-male-left";
    pEntData->animationSet.layers[WfToolAnimationLayer].animationNames[Right] = "walk-axe-male-right";

    struct Component2D* pComp = WfGetPlayerAnimationLayerComponent(pPlayer, WfToolAnimationLayer);
    WfSetPlayerOverlayAnimations(pEntData->directionFacing, pLayer, pEntData, pPlayer);
    pComp->data.spriteAnimator.onSprite = 0;
}

static void OnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    pEntData->animationSet.layersMask &= ~(1 << WfToolAnimationLayer);
    struct Component2D* pComp = WfGetPlayerAnimationLayerComponent(pPlayer, WfToolAnimationLayer);
    pComp->data.spriteAnimator.bDraw = false;
}

static bool OnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    return true;
}

static bool TryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot)
{
    return false;
}


static struct WfItemDef gDef = 
{
    .UISpriteName = "basic-axe",
    .pUserData = NULL,
    .onMakeCurrent = &OnMakeCurrentItem,
    .onStopBeingCurrent = &OnStopBeingCurrentItem,
    .onUseItem = &OnUseItem,
    .onTryEquip = &TryEquip
};

void WfAddBasicAxeDef()
{
    WfAddItemDef(&gDef);
}
