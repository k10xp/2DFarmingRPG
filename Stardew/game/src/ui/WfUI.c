#include "WfUI.h"
#include "GameFramework.h"
#include "XMLUIGameLayer.h"
#include "DrawContext.h"

void WfPushHUD(DrawContext* pDC)
{
    struct GameFrameworkLayer testLayer;
    memset(&testLayer, 0, sizeof(struct GameFrameworkLayer));
    struct XMLUIGameLayerOptions options;
    options.xmlPath = "./Assets/GameHUD.xml";
    options.pDc = pDC;
    testLayer.flags |= (EnableOnPush | EnableOnPop);
    printf("making xml ui layer\n");
    XMLUIGameLayer_Get(&testLayer, &options);
    printf("done\n");
    printf("pushing framework layer\n");
    GF_PushGameFrameworkLayer(&testLayer);
}

void WfPushSettings(DrawContext* pDC)
{
    struct GameFrameworkLayer testLayer;
    memset(&testLayer, 0, sizeof(struct GameFrameworkLayer));
    struct XMLUIGameLayerOptions options;
    options.xmlPath = "./Assets/Settings.xml";
    options.pDc = pDC;
    printf("making xml ui layer\n");
    XMLUIGameLayer_Get(&testLayer, &options);
    testLayer.flags |= ( MasksUpdate | MasksInput );

    printf("done\n");
    printf("pushing framework layer\n");
    GF_PushGameFrameworkLayer(&testLayer);
}
