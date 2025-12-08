#include "GameFrameworkEvent.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "StringKeyHashMap.h"
#include "AssertLib.h"
#include "Log.h"

#define EVENT_MAX_NAME_LEN 64

static struct HashMap gEventMap;

struct GameFrameworkEventListener
{
	void* userData;
	EventListenerFn eventFn;
	struct GameFrameworkEventListener* pNext;
	struct GameFrameworkEventListener* pPrev;
	char eventName[EVENT_MAX_NAME_LEN];
};

struct GameFrameworkEvent
{
	struct GameFrameworkEventListener* pListenerHead;
	struct GameFrameworkEventListener* pListenerTail;
};

static struct GameFrameworkEventListener* AddListener(struct GameFrameworkEvent* pEvent, EventListenerFn listenerFn, void* pUser, char* eventName)
{
	struct GameFrameworkEventListener* pListener = malloc(sizeof(struct GameFrameworkEventListener));
	EASSERT(pListener);
	memset(pListener, 0, sizeof(struct GameFrameworkEventListener));
	strcpy(pListener->eventName, eventName);
	pListener->eventFn = listenerFn;
	pListener->userData = pUser;
	if (!pEvent->pListenerTail)
	{
		EASSERT(!pEvent->pListenerHead);
		pEvent->pListenerTail = pListener;
		pEvent->pListenerHead = pListener;
		pListener->pNext = NULL;
		pListener->pPrev = NULL;
	}
	else
	{
		pEvent->pListenerTail->pNext = pListener;
		pListener->pPrev = pEvent->pListenerTail;
		pListener->pNext = NULL;
		pEvent->pListenerTail = pListener;
	}
	return pListener;
}

struct GameFrameworkEventListener* Ev_SubscribeEvent(char* eventName, EventListenerFn listenerFn, void* pUser)
{
	Log_Verbose("SUBSCRIBING TO EVENT %s", eventName);
	EASSERT(strlen(eventName) < EVENT_MAX_NAME_LEN);
	struct GameFrameworkEvent* pEvent = HashmapSearch(&gEventMap, eventName);
	if (pEvent == NULL)
	{
		struct GameFrameworkEvent ev;
		memset(&ev, 0, sizeof(struct GameFrameworkEvent));
		pEvent = HashmapInsert(&gEventMap, eventName, &ev);
	}
	return AddListener(pEvent, listenerFn, pUser, eventName);
}

bool Ev_UnsubscribeEvent(struct GameFrameworkEventListener* pListener)
{
	Log_Verbose("UNSUBSCRIBING TO EVENT %s", pListener->eventName);
	struct GameFrameworkEvent* pEvent = HashmapSearch(&gEventMap, pListener->eventName);
	if (pEvent)
	{
		if (pEvent->pListenerHead == pListener)
		{
			pEvent->pListenerHead = pListener->pNext;
		}
		if (pEvent->pListenerTail == pListener)
		{
			pEvent->pListenerTail = pListener->pPrev;
		}
		if (pListener->pNext)
		{
			pListener->pNext->pPrev = pListener->pPrev;
		}
		if (pListener->pPrev)
		{
			pListener->pPrev->pNext = pListener->pNext;
		}
		free(pListener);
		return true;
	}
	else
	{
		Log_Warning("Ev_UnsubscribeEvent failed. Event name '%s'", pListener->eventName);
	}
	return false;
}

void Ev_FireEvent(char* eventName, void* eventArgs)
{
	int i = 0;
	Log_Verbose("FIRING EVENT %s", eventName);
	struct GameFrameworkEvent* pEvent = HashmapSearch(&gEventMap, eventName);
	if (pEvent)
	{
		struct GameFrameworkEventListener* pListener = pEvent->pListenerHead;
		while (pListener)
		{
			pListener->eventFn(pListener->userData, eventArgs);
			pListener = pListener->pNext;
			Log_Verbose("%s DEBUG COUNT %i", eventName, i++);
		}
	}
}

void Ev_Init()
{
	HashmapInit(&gEventMap, 16, sizeof(struct GameFrameworkEvent));
}

void* Ev_GetUserData(struct GameFrameworkEventListener* pListener)
{
	return pListener->userData;
}