#include <pebble.h>
#include "GlobalState.h"
#include "Logging.h"

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
static int globalStateInstanceCount = 0;

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

	if(globalStateInstanceCount > 0)
	{
		GlobalStateInstance *oldInstance = &globalStateInstances[globalStateInstanceCount - 1];
		if(oldInstance->disappearCallback)
			oldInstance->disappearCallback(oldInstance->data);
	}
	
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
	DEBUG_LOG("appearcallback for %d", instance->state);
	if(instance->appearCallback)
		instance->appearCallback(instance->data);
}

void UpdateGlobalState(TimeUnits units_changed)
{
	if(globalStateInstanceCount <= 0)
		return;
	
	GlobalStateInstance *instance = &globalStateInstances[globalStateInstanceCount - 1];
	DEBUG_LOG("update global state: %d", instance->state);
	
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

	if(globalStateInstanceCount == 0)
	{
		window_stack_pop(true);
		return;
	}
	
	GlobalStateInstance *newInstance = &globalStateInstances[globalStateInstanceCount - 1];
	DEBUG_LOG("reveal global state: %d", newInstance->state);
	
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
