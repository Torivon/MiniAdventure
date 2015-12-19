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

void PushGlobalState(GlobalState state, 
					 TimeUnits triggerUnits,
					 GlobalStateChangeCallback updateCallback,
					 GlobalStateChangeCallback pushCallback, 
					 GlobalStateChangeCallback appearCallback, 
					 GlobalStateChangeCallback disappearCallback, 
					 GlobalStateChangeCallback popCallback,
					 void *data);

void UpdateGlobalState(TimeUnits units_changed);
void PopGlobalState(void);
GlobalState GetCurrentGlobalState(void);
void PopAllGlobalStates(void);

