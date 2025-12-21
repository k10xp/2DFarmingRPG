#ifndef WFVERSION_H
#define WFVERSION_H

#define WF_MAJOR_VERSION "0"
#define WF_MINOR_VERSION "0"
#define WF_REVISION "0"

#define WF_BRANCH "main"
#define BUILD_SHA "<local_build>"
#define BUILD_TIME "<local_build>"

const char* WfGetVersionString();

#endif