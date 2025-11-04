#ifndef WFPLAYER_H
#define WFPLAYER_H

#include <cglm/cglm.h>
#include "WfEnums.h"

struct BinarySerializer;
struct Entity2D;
struct GameLayer2DData;

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

void WfInitPlayer();

void WfDeSerializePlayerEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData);

void WfSerializePlayerEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData);

void WfMakeIntoPlayerEntity(struct Entity2D* pInEnt, struct GameLayer2DData* pData, vec2 spawnAtGroundPos);

#endif