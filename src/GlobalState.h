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
    STATE_STATE_POP,
    STATE_RESET_GAME,
    STATE_UPDATE_GAME_STATE,
    STATE_ENGINE_MENU,
    STATE_REGISTER_MENU_STATE,
} GlobalState;

typedef void(*GlobalStateQueueFunction)(void);

void GlobalState_QueueStatePop(void);

// Pushes a new state immediately. Should be used when opening a menu or
// Switching game modes
void GlobalState_Push(GlobalState state,
					 TimeUnits triggerUnits,
					 void *data);

// Pushes a new state on the next pop. This allows for sibling state transitions.
// Used for sequential dialogs, menus, or battles.
void GlobalState_Queue(GlobalState state,
                      TimeUnits triggerUnits,
                      void *data);

void GlobalState_ClearQueue(void);

void GlobalState_Update(TimeUnits units_changed);
void GlobalState_Pop(void);
void GlobalState_PopIgnoreQueue(void);
GlobalState GlobalState_GetCurrent(void);
void GlobalState_PopAll(void);
void GlobalState_Initialize(void);
void GlobalState_Free(void);


