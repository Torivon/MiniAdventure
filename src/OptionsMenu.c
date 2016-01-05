#include <pebble.h>

#include "GlobalState.h"
#include "BaseWindow.h"
#include "Menu.h"
#include "OptionsMenu.h"
#include "WorkerControl.h"

void DrawOptionsMenu(void);

static bool vibration = true;
static bool useWorkerApp = false;
static bool workerCanLaunch = true;

void ToggleVibration(void)
{
	vibration = !vibration;
	DrawOptionsMenu();
}

bool GetVibration(void)
{
	return vibration;
}

void SetVibration(bool enable)
{
	vibration = enable;
}

void SetWorkerApp(bool enable)
{
	useWorkerApp = enable;
	if(OptionsMenuIsVisible())
		DrawOptionsMenu();
}

void ToggleWorkerApp(void)
{
	if(useWorkerApp)
	{
		AttemptToKillWorkerApp();
	}
	else
	{
		AttemptToLaunchWorkerApp();
	}
}

bool GetWorkerApp(void)
{
	return useWorkerApp;
}

void SetWorkerCanLaunch(bool enable)
{
	workerCanLaunch = enable;
	SendWorkerCanLaunch();
}

void ToggleWorkerCanLaunch(void)
{
	if(!useWorkerApp)
		return;
	
	SetWorkerCanLaunch(!workerCanLaunch);
	DrawOptionsMenu();
}

bool GetWorkerCanLaunch(void)
{
	return workerCanLaunch;
}

bool OptionsMenuIsVisible(void)
{
	return GlobalState_GetCurrent() == STATE_OPTIONS;
}

void DrawOptionsMenu(void)
{
	ReloadMenu(GetMainMenu());
	ReloadMenu(GetSlaveMenu());
}

MenuCellDescription optionScreenMenuList[] = 
{
	{.name = "Vibration", .description = "Allow Vibration", .callback = NULL},
	{.name = "Background", .description = "Use worker app", .callback = NULL},
	{.name = "Launch", .description = "Worker app can launch", .callback = NULL},
};

static bool firstLaunch = false;

uint16_t OptionMenuCount(void)
{
	return sizeof(optionScreenMenuList)/sizeof(*optionScreenMenuList);
}

const char *OptionMainMenuNameCallback(int row)
{
	switch(row)
	{
		case 0:
		{
			if(GetVibration())
				return "On";
			else
				return "Off";
		}
		case 1:
		{
			if(GetWorkerApp())
				return "On";
			else
				return "Off";
		}
		case 2:
		{
			if(GetWorkerCanLaunch())
				return "On";
			else
				return "Off";
		}
	}
	return "";
}

const char *OptionMainMenuDescriptionCallback(int row)
{
	return optionScreenMenuList[row].description;
}

void OptionMainMenuSelectCallback(int row)
{
	switch(row)
	{
		case 0:
		{
			ToggleVibration();
			break;
		}
		case 1:
		{
			ToggleWorkerApp();
			break;
		}
		case 2:
		{
			ToggleWorkerCanLaunch();
			break;
		}
	}
}

void OptionScreenPush(void *data)
{
	firstLaunch = true;
}

void OptionScreenAppear(void *data)
{
	SetUseSlaveMenu(true);
	SetHideMenuOnSelect(false);	
	RegisterMenuCellCallbacks(GetMainMenu(), OptionMenuCount, OptionMainMenuNameCallback, OptionMainMenuDescriptionCallback, OptionMainMenuSelectCallback);	
	RegisterMenuCellList(GetSlaveMenu(), optionScreenMenuList, OptionMenuCount());
	if(firstLaunch)
	{
		TriggerMenu(GetMainMenu());
		ShowMenu(GetSlaveMenu());
		firstLaunch = false;
	}
	else
	{
		GlobalState_Pop();
	}
	
}

void OptionScreenPop(void *data)
{
	SetUseSlaveMenu(false);
	SetHideMenuOnSelect(true);
}

void TriggerOptionScreen(void)
{
	GlobalState_Push(STATE_OPTIONS, 0, NULL, OptionScreenPush, OptionScreenAppear, NULL, OptionScreenPop, NULL);
}

void QueueOptionsScreen(void)
{
    GlobalState_Queue(STATE_OPTIONS, 0, NULL, OptionScreenPush, OptionScreenAppear, NULL, OptionScreenPop, NULL);
}
