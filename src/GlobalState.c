#include <pebble.h>
#include "GlobalState.h"
#include "Logging.h"

typedef struct GlobalStateInstance GlobalStateInstance;

typedef struct GlobalStateInstance
{
	GlobalState state;
	TimeUnits triggerUnits;
	GlobalStateChangeCallback updateCallback;
	GlobalStateChangeCallback pushCallback;
	GlobalStateChangeCallback appearCallback;
	GlobalStateChangeCallback disappearCallback;
	GlobalStateChangeCallback popCallback;
	void *data;
    GlobalStateInstance *prev;
    GlobalStateInstance *next;
} GlobalStateInstance;

static GlobalStateInstance *stackTop = NULL;
static GlobalStateInstance *queueFront = NULL;
static GlobalStateInstance *queueBack = NULL;

//static GlobalStateInstance globalStateInstances[MAX_GLOBAL_STATES];
//static Queue *globalQueue = NULL;

//static int globalStateInstanceCount = 0;

void GlobalState_Queue(GlobalState state,
                      TimeUnits triggerUnits,
                      GlobalStateChangeCallback updateCallback,
                      GlobalStateChangeCallback pushCallback,
                      GlobalStateChangeCallback appearCallback,
                      GlobalStateChangeCallback disappearCallback,
                      GlobalStateChangeCallback popCallback,
                      void *data)
{
    GlobalStateInstance *newInstance = calloc(sizeof(GlobalStateInstance), 1);
    
    newInstance->state = state;
    newInstance->triggerUnits = triggerUnits;
    newInstance->updateCallback = updateCallback;
    newInstance->pushCallback = pushCallback;
    newInstance->appearCallback = appearCallback;
    newInstance->disappearCallback = disappearCallback;
    newInstance->popCallback = popCallback;
    newInstance->data = data;
    newInstance->next = NULL;
    newInstance->prev = NULL;
    
    if(queueBack == NULL)
    {
        queueFront = queueBack = newInstance;
    }
    else
    {
        queueBack->next = newInstance;
        newInstance->prev = queueBack;
        queueBack = newInstance;
    }
}

static void GlobalState_PushInternal(GlobalStateInstance *newInstance)
{
    newInstance->next = NULL;
    newInstance->prev = NULL;
    
    if(stackTop == NULL)
    {
        stackTop = newInstance;
    }
    else
    {
        stackTop->next = newInstance;
        newInstance->prev = stackTop;
        stackTop = newInstance;
    }
    
	if(newInstance->pushCallback)
		newInstance->pushCallback(newInstance->data);
	if(newInstance->appearCallback)
		newInstance->appearCallback(newInstance->data);
}

GlobalStateInstance *GlobalState_PopQueue(void)
{
    if(!queueBack)
        return NULL;
    
    GlobalStateInstance *instance = queueFront;
    queueFront = instance->next;
    if(queueFront)
    {
        queueFront->prev = NULL;
    }
    else
    {
        queueBack = NULL;
    }
    return instance;
}

void GlobalState_ClearQueue(void)
{
    while(queueBack)
    {
        GlobalStateInstance *instance = GlobalState_PopQueue();
        free(instance);
    }
}

static void PopAppear(void *data)
{
    GlobalState_Pop();
    GlobalState_Pop();
}

void GlobalState_QueueStatePop(void)
{
    GlobalState_Queue(STATE_STATE_POP, 0, NULL, PopAppear, NULL, NULL, NULL, NULL);
}

void GlobalState_Push(GlobalState state,
                                     TimeUnits triggerUnits,
                                     GlobalStateChangeCallback updateCallback,
                                     GlobalStateChangeCallback pushCallback,
                                     GlobalStateChangeCallback appearCallback,
                                     GlobalStateChangeCallback disappearCallback, 
                                     GlobalStateChangeCallback popCallback,
                                     void *data)
{
    DEBUG_LOG("Push global state: %d", state);
    // Pushing a new instance clears the queue
    
    GlobalState_ClearQueue();
    
    if(stackTop)
    {
        if(stackTop->disappearCallback)
            stackTop->disappearCallback(stackTop->data);
    }
    
    GlobalStateInstance *newInstance = calloc(sizeof(GlobalStateInstance), 1);
    
    newInstance->state = state;
    newInstance->triggerUnits = triggerUnits;
    newInstance->updateCallback = updateCallback;
    newInstance->pushCallback = pushCallback;
    newInstance->appearCallback = appearCallback;
    newInstance->disappearCallback = disappearCallback;
    newInstance->popCallback = popCallback;
    newInstance->data = data;

    GlobalState_PushInternal(newInstance);
}

void GlobalState_Update(TimeUnits units_changed)
{
	if(!stackTop)
		return;

	if(stackTop->updateCallback && (units_changed & stackTop->triggerUnits))
	{
		stackTop->updateCallback(stackTop->data);
	}
}

void GlobalState_Pop(void)
{
    if(!stackTop)
        return;
	
    GlobalStateInstance *oldInstance = stackTop;
    
	DEBUG_LOG("pop global state: %d", oldInstance->state);
	if(oldInstance->disappearCallback)
		oldInstance->disappearCallback(oldInstance->data);
		
	if(oldInstance->popCallback)
		oldInstance->popCallback(oldInstance->data);
	
    stackTop = oldInstance->prev;
    if(stackTop)
        stackTop->next = NULL;
    
    free(oldInstance);
    
    if(queueBack)
    {
        GlobalStateInstance *queuedInstance = GlobalState_PopQueue();
        if(queuedInstance)
        {
            GlobalState_PushInternal(queuedInstance);
            return;
        }
    }
    
	if(!stackTop)
	{
        window_stack_pop(true);
		return;
	}

	if(stackTop->appearCallback)
		stackTop->appearCallback(stackTop->data);
}

GlobalState GlobalState_GetCurrent(void)
{
    if(!stackTop)
        return STATE_NONE;
	
	return stackTop->state;
}

void GlobalState_PopAll(void)
{
	while(stackTop)
	{
		GlobalState_Pop();
	}
}

void GlobalState_Initialize(void)
{
}

void GlobalState_Free(void)
{
    GlobalState_PopAll();
}
