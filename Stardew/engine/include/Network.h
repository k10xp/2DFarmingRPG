#ifndef NETWORK_H
#define NETWORK_H

struct HostInfo
{
    const char* ip;
    unsigned short port;
};

enum GameRole
{
    GR_Singleplayer,
    GR_Client,
    GR_ClientServer
};

void NW_Init();

#endif