#pragma once

typedef void(*GlobalStateChangeCallback)(void *data);

typedef enum
{
	STATE_NONE = 0,
	STATE_TITLE_SCREEN,
	STATE_ADVENTURE,
	STATE_BATTLE,
	STATE_VICTORY,
	STATE_LOSS,
	STATE_DIALOG,
	STATE_MENU,
	STATE_OPTIONS,
	STATE_LARGE_IMAGE,
} GlobalState;

typedef void(*GlobalStateQueueFunction)(void);

// Pushes a new state immediately. Should be used when opening a menu or
// Switching game modes
void PushGlobalState(GlobalState state, 
					 TimeUnits triggerUnits,
					 GlobalStateChangeCallback updateCallback,
					 GlobalStateChangeCallback pushCallback, 
					 GlobalStateChangeCallback appearCallback, 
					 GlobalStateChangeCallback disappearCallback, 
					 GlobalStateChangeCallback popCallback,
					 void *data);

// Pushes a new state on the next pop. This allows for sibling state transitions.
// Used for sequential dialogs, menus, or battles.
void QueueGlobalState(GlobalState state,
                      TimeUnits triggerUnits,
                      GlobalStateChangeCallback updateCallback,
                      GlobalStateChangeCallback pushCallback,
                      GlobalStateChangeCallback appearCallback,
                      GlobalStateChangeCallback disappearCallback,
                      GlobalStateChangeCallback popCallback,
                      void *data);

void ClearGlobalStateQueue(void);

void UpdateGlobalState(TimeUnits units_changed);
void PopGlobalState(void);
GlobalState GetCurrentGlobalState(void);
void PopAllGlobalStates(void);
void GlobalState_Initialize(void);
void GlobalState_Free(void);


