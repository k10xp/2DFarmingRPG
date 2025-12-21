#include "WfVersion.h"

const char* WfGetVersionString()
{
    static const char* s = "v" WF_MAJOR_VERSION "." WF_MINOR_VERSION "." WF_REVISION "-" WF_BRANCH "-" BUILD_SHA "-" BUILD_TIME;
    return s;
}