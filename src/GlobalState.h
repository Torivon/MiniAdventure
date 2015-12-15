#pragma once

typedef void(*GlobalStateChangeCallback)(void);

typedef enum
{
	STATE_NONE = 0,
	TITLE_SCREEN,
	ADVENTURE,
	BATTLE,
	VICTORY,
	LOSS,
	DIALOG,
	MENU,
} GlobalState;

void PushGlobalState(GlobalState state, 
				  TimeUnits triggerUnits,
				  GlobalStateChangeCallback updateCallback,
				  GlobalStateChangeCallback pushCallback, 
				  GlobalStateChangeCallback appearCallback, 
				  GlobalStateChangeCallback disappearCallback, 
				  GlobalStateChangeCallback popCallback);

void UpdateGlobalState(TimeUnits units_changed);
void PopGlobalState(void);
GlobalState GetCurrentGlobalState(void);
void PopAllGlobalStates(void);

