#include <pebble.h>

#include "GlobalState.h"
#include "BaseWindow.h"
#include "Menu.h"
#include "OptionsMenu.h"
#include "WorkerControl.h"

void ReloadOptionsMenu(void);

static bool vibration = true;
static bool useWorkerApp = false;
static bool workerCanLaunch = true;
static bool allowActivity = true;

void ToggleVibration(void)
{
	vibration = !vibration;
	ReloadOptionsMenu();
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
    ReloadOptionsMenu();
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
}

void ToggleWorkerCanLaunch(void)
{
	if(!useWorkerApp)
		return;
	
	SetWorkerCanLaunch(!workerCanLaunch);
	ReloadOptionsMenu();
}

bool GetWorkerCanLaunch(void)
{
	return workerCanLaunch;
}

void ToggleActivity(void)
{
    allowActivity = !allowActivity;
    ReloadOptionsMenu();
}

bool GetAllowActivity(void)
{
    return allowActivity;
}

void SetAllowActivity(bool enable)
{
    allowActivity = enable;
}

bool OptionsMenuIsVisible(void)
{
	return GlobalState_GetCurrent() == STATE_OPTIONS;
}

void ReloadOptionsMenu(void)
{
    ReloadMenu(GetMainMenu());
    ReloadMenu(GetSlaveMenu());
}

static bool firstLaunch = false;

uint16_t OptionsMenu_CellCount(uint16_t sectionIndex)
{
    return 4;
}

const char *OptionsMenu_SlaveCellName(MenuIndex *index)
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
        case 3:
        {
            return "Activity";
        }
    }
    return "";
}

const char *OptionsMenu_CellDescription(MenuIndex *index)
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
        case 3:
        {
            return "Use activity to determine path progress";
        }
    }
    return "";
}

const char *OptionsMenu_MainCellName(MenuIndex *index)
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
        case 3:
        {
            if(GetAllowActivity())
                return "On";
            else
                return "Off";
        }
	}
	return "";
}

const char *OptionsMenu_SectionName(uint16_t sectionIndex)
{
    return "Options";
}

uint16_t OptionsMenu_SectionCount(void)
{
    return 1;
}

void OptionsMenu_Select(MenuIndex *index)
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
        case 3:
        {
            ToggleActivity();
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
    RegisterMenuState(GetMainMenu(), STATE_OPTIONS);
    RegisterMenuState(GetSlaveMenu(), STATE_OPTIONS);
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
