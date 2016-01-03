#include <pebble.h>
#include "GlobalState.h"
#include "Logging.h"
#include "Queue.h"

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
} GlobalStateInstance;

#define MAX_GLOBAL_STATES 20

static GlobalStateInstance globalStateInstances[MAX_GLOBAL_STATES];
static Queue *globalQueue = NULL;
static int globalStateInstanceCount = 0;

void QueueGlobalState(GlobalState state,
                      TimeUnits triggerUnits,
                      GlobalStateChangeCallback updateCallback,
                      GlobalStateChangeCallback pushCallback,
                      GlobalStateChangeCallback appearCallback,
                      GlobalStateChangeCallback disappearCallback,
                      GlobalStateChangeCallback popCallback,
                      void *data)
{
    if(Queue_IsFull(globalQueue))
        return;
    
    GlobalStateInstance *newInstance = calloc(sizeof(GlobalStateInstance), 1);
    
    newInstance->state = state;
    newInstance->triggerUnits = triggerUnits;
    newInstance->updateCallback = updateCallback;
    newInstance->pushCallback = pushCallback;
    newInstance->appearCallback = appearCallback;
    newInstance->disappearCallback = disappearCallback;
    newInstance->popCallback = popCallback;
    newInstance->data = data;
    
    Queue_Push(globalQueue, newInstance);
}

static void PushGlobalState_internal(GlobalState state,
					 TimeUnits triggerUnits, 
					 GlobalStateChangeCallback updateCallback, 
					 GlobalStateChangeCallback pushCallback, 
					 GlobalStateChangeCallback appearCallback, 
					 GlobalStateChangeCallback disappearCallback, 
					 GlobalStateChangeCallback popCallback,
					 void *data)
{
	GlobalStateInstance *instance = &globalStateInstances[globalStateInstanceCount];
	globalStateInstanceCount++;
	
	instance->state = state;	
	instance->triggerUnits = triggerUnits;
	instance->updateCallback = updateCallback;
	instance->pushCallback = pushCallback;
	instance->appearCallback = appearCallback;
	instance->disappearCallback = disappearCallback;
	instance->popCallback = popCallback;
	instance->data = data;
	if(instance->pushCallback)
		instance->pushCallback(instance->data);
	if(instance->appearCallback)
		instance->appearCallback(instance->data);
}

void GlobalState_ClearQueue(void)
{
    while(!Queue_IsEmpty(globalQueue))
    {
        GlobalStateInstance *instance = Queue_Pop(globalQueue);
        free(instance);
    }
}

static void PopAppear(void *data)
{
    PopGlobalState();
    PopGlobalState();
}

void GlobalState_QueueStatePop(void)
{
    QueueGlobalState(STATE_STATE_POP, 0, NULL, PopAppear, NULL, NULL, NULL, NULL);
}

void PushGlobalState(GlobalState state,
                                     TimeUnits triggerUnits,
                                     GlobalStateChangeCallback updateCallback,
                                     GlobalStateChangeCallback pushCallback,
                                     GlobalStateChangeCallback appearCallback,
                                     GlobalStateChangeCallback disappearCallback, 
                                     GlobalStateChangeCallback popCallback,
                                     void *data)
{
    DEBUG_LOG("Push global state: %d", state);
    if(globalStateInstanceCount == MAX_GLOBAL_STATES)
        return;
    
    // Pushing a new instance clears the queue
    
    GlobalState_ClearQueue();
    
    if(globalStateInstanceCount > 0)
    {
        GlobalStateInstance *oldInstance = &globalStateInstances[globalStateInstanceCount - 1];
        if(oldInstance->disappearCallback)
            oldInstance->disappearCallback(oldInstance->data);
    }
    
    PushGlobalState_internal(state, triggerUnits, updateCallback, pushCallback, appearCallback, disappearCallback, popCallback, data);
}

void UpdateGlobalState(TimeUnits units_changed)
{
	if(globalStateInstanceCount <= 0)
		return;
	
	GlobalStateInstance *instance = &globalStateInstances[globalStateInstanceCount - 1];

	if(instance->updateCallback && (units_changed & instance->triggerUnits))
	{
		instance->updateCallback(instance->data);
	}
}

void PopGlobalState(void)
{
	if(globalStateInstanceCount == 0)
	{
		return;
	}
	
	GlobalStateInstance *oldInstance = &globalStateInstances[globalStateInstanceCount - 1];
	DEBUG_LOG("pop global state: %d", oldInstance->state);
	if(oldInstance->disappearCallback)
		oldInstance->disappearCallback(oldInstance->data);
		
	if(oldInstance->popCallback)
		oldInstance->popCallback(oldInstance->data);
	
	globalStateInstanceCount--;

    if(!Queue_IsEmpty(globalQueue))
    {
        GlobalStateInstance *queuedInstance = Queue_Pop(globalQueue);
        if(queuedInstance)
        {
            PushGlobalState_internal(queuedInstance->state,
                            queuedInstance->triggerUnits,
                            queuedInstance->updateCallback,
                            queuedInstance->pushCallback,
                            queuedInstance->appearCallback,
                            queuedInstance->disappearCallback,
                            queuedInstance->popCallback,
                            queuedInstance->data);
            free(queuedInstance);
            return;
        }
    }
    
	if(globalStateInstanceCount == 0)
	{
		window_stack_pop(true);
		return;
	}
	
	GlobalStateInstance *newInstance = &globalStateInstances[globalStateInstanceCount - 1];
	
	if(newInstance->appearCallback)
		newInstance->appearCallback(newInstance->data);
}

GlobalState GetCurrentGlobalState(void)
{
	if(globalStateInstanceCount == 0)
		return STATE_NONE;
	
	return globalStateInstances[globalStateInstanceCount - 1].state;
}

void PopAllGlobalStates(void)
{
	while(globalStateInstanceCount > 0)
	{
		PopGlobalState();
	}
}

void GlobalState_Initialize(void)
{
    INFO_LOG("Initializing global state");
    if(!globalQueue)
    {
        globalQueue = Queue_Create(MAX_GLOBAL_STATES);
    }
}

void GlobalState_Free(void)
{
    if(globalQueue)
    {
        while(!Queue_IsEmpty(globalQueue))
        {
            GlobalStateInstance *instance = Queue_Pop(globalQueue);
            free(instance);
        }
        Queue_Free(globalQueue);
        globalQueue = NULL;
    }
}
