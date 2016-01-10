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

static bool firstLaunch = false;

uint16_t OptionMenuCount(uint16_t sectionIndex)
{
    return 3;
}

const char *OptionSlaveMenuNameCallback(MenuIndex *index)
{
    switch(index->row)
    {
        case 0:
        {
            return "Vibration";
        }
        case 1:
        {
            return "Background";
        }
        case 2:
        {
            return "Launch";
        }
    }
    return "";
}

const char *OptionMenuDescriptionCallback(MenuIndex *index)
{
    switch(index->row)
    {
        case 0:
        {
            return "Allow Vibration";
        }
        case 1:
        {
            return "Use worker app";
        }
        case 2:
        {
            return "Worker app can launch";
        }
    }
    return "";
}

const char *OptionMainMenuNameCallback(MenuIndex *index)
{
	switch(index->row)
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

static const char *OptionMenuSectionName(uint16_t sectionIndex)
{
    return "Options";
}

static uint16_t OptionMenuSectionCount(void)
{
    return 1;
}

void OptionMainMenuSelectCallback(MenuIndex *index)
{
	switch(index->row)
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

static MenuParameters mainParameters = {.menuSectionNameCallback = OptionMenuSectionName,
    .menuSectionCountCallback = OptionMenuSectionCount,
    .countCallback = OptionMenuCount,
    .nameCallback = OptionMainMenuNameCallback,
    .descriptionCallback = OptionMenuDescriptionCallback,
    .selectCallback = OptionMainMenuSelectCallback};
static MenuParameters slaveParameters = {.menuSectionNameCallback = NULL,
    .menuSectionCountCallback = OptionMenuSectionCount,
    .countCallback = OptionMenuCount,
    .nameCallback = OptionSlaveMenuNameCallback,
    .descriptionCallback = OptionMenuDescriptionCallback,
    .selectCallback = NULL};

void OptionScreenAppear(void *data)
{
	SetUseSlaveMenu(true);
	SetHideMenuOnSelect(false);	
	RegisterMenuCellCallbacks(GetMainMenu(), &mainParameters);
    RegisterMenuCellCallbacks(GetSlaveMenu(), &slaveParameters);
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
	GlobalState_Push(STATE_OPTIONS, 0, NULL);
}

void QueueOptionsScreen(void)
{
    GlobalState_Queue(STATE_OPTIONS, 0, NULL);
}
