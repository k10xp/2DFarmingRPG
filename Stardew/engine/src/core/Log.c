#include "Log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "Thread.h"

static CrossPlatformMutex gLogMtx;
static enum LogLvl gLogLevel = LogLvl_Verbose;

char* gLogLevelNames[LogLvl_NumLevels] = 
{
    "[Verbose] ",
    "[Info] ",
    "[Warning] ",
    "[Error] ",
};

void Log_SetLevel(enum LogLvl lvl)
{
    gLogLevel = lvl;
}

int Log_Init()
{
    InitMutex(&gLogMtx);
}

void Log_DeInit()
{
    DestroyMutex(&gLogMtx);
}

static int vLog_Fmt(const char* fmt, enum LogLvl lvl, va_list args)
{
    if(lvl < gLogLevel)
    {
        return 0;
    }
    char gLogBuffer[512];
    snprintf(gLogBuffer, 512, gLogLevelNames[lvl]);
    int namelen = strlen(gLogLevelNames[lvl]);
    char* start = gLogBuffer + namelen;
    vsnprintf(start, 512 - namelen, fmt, args);

    LockMutex(&gLogMtx);
    int r = printf("%s\n", gLogBuffer);
    UnlockMutex(&gLogMtx);
    return r;
}

int Log_Verbose(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = vLog_Fmt(fmt, LogLvl_Verbose, args);
    va_end(args);
    return r;
}

int Log_Fmt(const char* fmt, enum LogLvl lvl, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = vLog_Fmt(fmt, lvl, args);
    va_end(args);
    return r;
}

int Log_Info(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = vLog_Fmt(fmt, LogLvl_Info, args);
    va_end(args);
    return r;
}

int Log_Warning(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = vLog_Fmt(fmt, LogLvl_Warning, args);
    va_end(args);
    return r;
}

int Log_Error(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = vLog_Fmt(fmt, LogLvl_Error, args);
    va_end(args);
    return r;
}