#include "WfScriptFunctions.h"
#include "WfItem.h"
#include "WfUI.h"
#include "lua.h"
#include <lualib.h>
#include "Scripting.h"
#include "main.h"

static int L_PushHUDLayer(lua_State* L)
{
    DrawContext* pDC = GetDrawContext();
    WfPushHUD(pDC);
    return 0;
}

void WfRegisterScriptFunctions()
{
    WfRegisterItemScriptFunctions();
    Sc_RegisterCFunction("WfPushHUD", &L_PushHUDLayer);
}

