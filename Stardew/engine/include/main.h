#ifndef  MAIN_H
#define MAIN_H

#include "DrawContext.h"
#include "InputContext.h"
#include "Network.h"
#include <stdbool.h>

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

struct CommandLineArgs
{
    enum GameRole role;
    char* serverAddress;
    char* clientAddress;
    bool bLogTextColoured;
    bool bIncludeLogTimeStamps;
    const char* logfilePath;
};

extern struct CommandLineArgs gCmdArgs;

#endif // ! MAIN_H
