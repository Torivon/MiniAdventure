#pragma once

typedef struct Queue Queue;

Queue *Queue_Create(int maxSize);
void Queue_Free(Queue *queue);
int Queue_GetSize(Queue *queue);
bool Queue_IsEmpty(Queue *queue);
bool Queue_IsFull(Queue *queue);
void Queue_Push(Queue* queue, void *data);
void *Queue_Pop(Queue *queue);
