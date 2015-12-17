#include <pebble.h>

#include "GlobalState.h"
#include "Menu.h"
#include "NewBaseWindow.h"
#include "NewMenu.h"
#include "OptionsMenu.h"
#include "UILayers.h"
#include "WorkerControl.h"

#define USE_NEW_OPTIONS_MENU 1

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

#if USE_NEW_OPTIONS_MENU

bool OptionsMenuIsVisible(void)
{
	return GetCurrentGlobalState() == STATE_OPTIONS;
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
	{.name = "Launch", .description = "Worker app can launch", .callback = NULL}
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
		PopGlobalState();
	}
	
}

void OptionScreenPop(void *data)
{
	SetUseSlaveMenu(false);
	SetHideMenuOnSelect(true);
}

void TriggerOptionScreen(void)
{
	PushGlobalState(STATE_OPTIONS, 0, NULL, OptionScreenPush, OptionScreenAppear, NULL, OptionScreenPop, NULL);
}

void ShowOptionsMenu(void)
{
	
}

#else


void DrawOptionsMenu(void)
{
	ShowMainWindowRow(0, "Options", "");
	ShowMainWindowRow(1, "Vibration", vibration ? "On" : "Off");
	ShowMainWindowRow(2, "Fast Mode", useWorkerApp ? "-" : fastMode ? "On" : "Off");
#if ALLOW_WORKER_APP
	ShowMainWindowRow(3, "Background", useWorkerApp ? "On" : "Off");
	ShowMainWindowRow(4, "Launch", !useWorkerApp ? "-" : workerCanLaunch ? "On" : "Off");
#endif
}

void OptionsMenuAppear(Window *window);
void OptionsMenuDisappear(Window *window);

MenuDefinition optionsMenuDef = 
{
	.menuEntries = 
	{
		{.text = "Quit", .description = "Return to main menu", .menuFunction = PopMenu},
		{.text = "Toggle", .description = "Toggle Vibration", .menuFunction = ToggleVibration},
		{.text = "Toggle", .description = "Speed up events", .menuFunction = ToggleFastMode},
#if ALLOW_WORKER_APP
		{.text = "Toggle", .description = "Run in background", .menuFunction = ToggleWorkerApp},
		{.text = "Toggle", .description = "Launch from background", .menuFunction = ToggleWorkerCanLaunch}
#endif
	},
	.appear = OptionsMenuAppear,
	.disappear = OptionsMenuDisappear,
	.mainImageId = -1,
	.floorImageId = -1
};

void OptionsMenuAppear(Window *window)
{
	MenuAppear(window);
	DrawOptionsMenu();
	optionsMenuVisible = true;
}

void OptionsMenuDisappear(Window *window)
{
	MenuDisappear(window);
	optionsMenuVisible = false;
}

void ShowOptionsMenu(void)
{
	PushNewMenu(&optionsMenuDef);
}

bool OptionsMenuIsVisible(void)
{
	return optionsMenuVisible;
}
#endif