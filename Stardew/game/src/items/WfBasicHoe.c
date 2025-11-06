#include "WfBasicHoe.h"
#include "WfItem.h"
#include <stdlib.h>
#include "WfItem.h"
#include "WfPlayer.h"
#include "Entities.h"

#include <stdlib.h>


static void OnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    pEntData->animationSet.layersMask |= (1 << WfToolAnimationLayer);
    pEntData->animationSet.layers[WfToolAnimationLayer].animationNames[Up] = "walk-hoe-male-up";
    pEntData->animationSet.layers[WfToolAnimationLayer].animationNames[Down] = "walk-hoe-male-down";
    pEntData->animationSet.layers[WfToolAnimationLayer].animationNames[Left] = "walk-hoe-male-left";
    pEntData->animationSet.layers[WfToolAnimationLayer].animationNames[Right] = "walk-hoe-male-right";

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
    .UISpriteName = "basic-hoe",
    .pUserData = NULL,
    .onMakeCurrent = &OnMakeCurrentItem,
    .onStopBeingCurrent = &OnStopBeingCurrentItem,
    .onUseItem = &OnUseItem,
    .onTryEquip = &TryEquip
};

void WfAddBasicHoeDef()
{
    WfAddItemDef(&gDef);
}
