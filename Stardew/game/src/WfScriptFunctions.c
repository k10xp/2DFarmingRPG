#include "WfScriptFunctions.h"
#include "WfItem.h"
#include "WfUI.h"
#include "lua.h"
#include <lualib.h>
#include "Scripting.h"
#include "main.h"
#include "Game2DLayer.h"
#include "WfPersistantGameData.h"
#include "Log.h"
#include "WfPlayerStart.h" /*WfGetCurrentLocalPlayer*/
#include "Entities.h"
#include "WfPlayer.h"

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
        Log_Error("L_SavePreferences ERROR");
    }
    else if(lua_islightuserdata(L, -1))
    {
        struct GameLayer2DData* pGL2D = lua_topointer(L, -1);
        struct WfPlayerPreferences* pPrefs = WfGetPreferences();
        pPrefs->zoomLevel = pGL2D->camera.scale[0];
    }
    else
    {
        Log_Error("L_SavePreferences ERROR");
    }
    return 0;
}

static int L_GetPlayerLocation(lua_State* L)
{
    if(lua_gettop(L) != 1)
    {
        Log_Error("L_GetPlayerLocation ERROR");
    }
    if(!lua_islightuserdata(L, 1))
    {
        Log_Error("L_GetPlayerLocation argument needs to be Game2DLayerData ptr");
        lua_pushnumber(L,0);
        lua_pushnumber(L,0);
        return 2;
    }
    HEntity2D hPlayer = WfGetCurrentLocalPlayer();
    vec2 pos = { 0.0f, 0.0f };

    if(hPlayer != NULL_HANDLE)
    {
        struct GameLayer2DData* pGL2D = lua_topointer(L, 1);
        struct Entity2D* pPlayer = Et2D_GetEntity(&pGL2D->entities, hPlayer);
        WfPlayerGetGroundContactPoint(pPlayer, pos);
    }
    else
    {
        Log_Error("L_GetPlayerLocation hPlayer != NULL_HANDLE");
    }
    lua_pushnumber(L,pos[0]);
    lua_pushnumber(L,pos[1]);
    return 2;
}

void WfRegisterScriptFunctions()
{
    WfRegisterItemScriptFunctions();
    Sc_RegisterCFunction("WfPushHUD", &L_PushHUDLayer);
    Sc_RegisterCFunction("WfSavePreferences", &L_SavePreferences);
    Sc_RegisterCFunction("WfGetPlayerLocation", &L_GetPlayerLocation);
}

