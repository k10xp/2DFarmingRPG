#ifndef  MAIN_H

#include "DrawContext.h"
#include "InputContext.h"

int Mn_GetScreenWidth();
int Mn_GetScreenHeight();

typedef void(*GameInitFn)(InputContext*,DrawContext*);

int EngineStart(int argc, char** argv, GameInitFn init);


/*
    TODO:
    Change functions which pass pointers to these to just use these global getters
*/
DrawContext* GetDrawContext();
InputContext* GetInputContext();

#endif // ! MAIN_H
