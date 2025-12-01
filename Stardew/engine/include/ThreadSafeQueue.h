#ifndef THREADSAFE_QUEUE_H
#define THREADSAFE_QUEUE_H

#include "IntTypes.h"
#include "Thread.h"
#include <stdbool.h>

/*
    Fixed size queue that wraps around when full
*/


/*
    callback for when an item is about to be lost by the queue reaching its maximum size
    and wrapping around, losing the first item in the queue. An opportunity to properly fre
    the item in the queue.
*/
typedef void(*OnTSQueueWrapAroundFn)(void* pItemToBeLost);


struct ThreadSafeQueue
{
    u8* data;
    u32 itemSize;
    u32 queueSize;
    u32 queueCurrentLength;
    u8* queueHead;
    u8* queueTail;
    CrossPlatformMutex mutex;
    OnTSQueueWrapAroundFn onWrap;
};


void TSQ_Init(struct ThreadSafeQueue* pQueue, u32 itemSize, u32 queueSizeItems, OnTSQueueWrapAroundFn wrapAroundCallback);

void TSQ_Enqueue(struct ThreadSafeQueue* pQueue, const void* pIn);

/*
    true if something dequeued, false if queue is empty
*/
bool TSQ_Dequeue(struct ThreadSafeQueue* pQueue, void* pOut);

#endif