#include "WfScriptFunctions.h"
#include "WfItem.h"
#include "WfUI.h"
#include "lua.h"
#include <lualib.h>
#include "Scripting.h"
#include "main.h"
#include "Game2DLayer.h"
#include "WfPersistantGameData.h"

static int L_PushHUDLayer(lua_State* L)
{
    DrawContext* pDC = GetDrawContext();
    WfPushHUD(pDC);
    return 0;
}

static int L_SavePreferences(lua_State* L)
{
    if(lua_gettop(L) != 1)
    {
        printf("L_SavePreferences ERROR\n");
    }
    else if(lua_islightuserdata(L, -1))
    {
        struct GameLayer2DData* pGL2D = lua_topointer(L, -1);
        struct WfPlayerPreferences* pPrefs = WfGetPreferences();
        pPrefs->zoomLevel = pGL2D->camera.scale[0];
    }
    else
    {
        printf("L_SavePreferences ERROR\n");
    }
    return 0;
}

void WfRegisterScriptFunctions()
{
    WfRegisterItemScriptFunctions();
    Sc_RegisterCFunction("WfPushHUD", &L_PushHUDLayer);
    Sc_RegisterCFunction("WfSavePreferences", &L_SavePreferences);
}

