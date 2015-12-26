#include <pebble.h>
#include "Queue.h"
#include "Logging.h"
#include "MiniAdventure.h"

typedef struct Queue
{
    int start;
    int end;
    void **dataArray;
    int arraySize;
    int maxSize;
} Queue;

Queue *Queue_Create(int maxSize)
{
    Queue *queue = calloc(sizeof(Queue), 1);
    queue->maxSize = maxSize;
    queue->arraySize = maxSize + 1; //allocating an extra slot for bookkeeping
    queue->start = queue->end = 0;
    queue->dataArray = calloc(sizeof(void*), queue->arraySize);
    return queue;
}

void Queue_Free(Queue *queue)
{
    if(!queue)
        return;
    
    free(queue->dataArray);
    free(queue);
}

bool Queue_IsEmpty(Queue *queue)
{
    return Queue_GetSize(queue) == 0;
}

bool Queue_IsFull(Queue *queue)
{
    return Queue_GetSize(queue) == queue->maxSize;
}

int Queue_GetSize(Queue *queue)
{
    int temp = queue->end - queue->start % queue->arraySize;
    if(temp < 0)
        temp += queue->arraySize;
    
    return temp;
}

void Queue_Push(Queue* queue, void *data)
{
    if(Queue_IsFull(queue))
        return;
    
    queue->dataArray[queue->end] = data;
    queue->end++;
    queue->end %= queue->arraySize;
}

void *Queue_Pop(Queue *queue)
{
    if(Queue_IsEmpty(queue))
        return NULL;
    
    void *data = queue->dataArray[queue->start];
    queue->dataArray[queue->start] = NULL;
    queue->start++;
    queue->start %= queue->arraySize;
    return data;
}
