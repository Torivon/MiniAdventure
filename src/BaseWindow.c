#include <pebble.h>
#include "Clock.h"
#include "DescriptionFrame.h"
#include "DialogFrame.h"
#include "GlobalState.h"
#include "MainImage.h"
#include "MenuArrow.h"
#include "MiniAdventure.h"
#include "BaseWindow.h"
#include "Battle.h"
#include "Menu.h"
#include "ProgressBar.h"
#include "Logging.h"
#include "Utils.h"

static Menu *mainMenu = NULL;
static Menu *slaveMenu = NULL;
static bool useSlaveMenu = false;
static bool hideMenuOnSelect = true;

static ProgressBar *batteryBar = NULL;
static uint16_t currentBatteryLevel = 0;
static uint16_t maxBatteryLevel = 100;
#if defined(PBL_ROUND)
#define BATTERY_FRAME {.origin = {.x = 122, .y = 134}, .size = {.w = 10, .h = 30}}
#else
#define BATTERY_FRAME {.origin = {.x = 20, .y = 61}, .size = {.w = 16, .h = 36}}
#endif

Menu *GetMainMenu(void)
{
	return mainMenu;
}

Menu *GetSlaveMenu(void)
{
	return slaveMenu;
}

void SetUseSlaveMenu(bool enable)
{
	useSlaveMenu = enable;
}

bool GetUseSlaveMenu(void)
{
	return useSlaveMenu;
}

void SetHideMenuOnSelect(bool enable)
{
	hideMenuOnSelect = enable;
}

bool GetHideMenuOnSelect(void)
{
	return hideMenuOnSelect;
}

void UpdateBatteryLevel(BatteryChargeState chargeState)
{
	currentBatteryLevel = chargeState.charge_percent;
	MarkProgressBarDirty(batteryBar);
}

void HideBatteryLevel(void)
{
	HideProgressBar(batteryBar);
}

void ShowBatteryLevel(void)
{
	ShowProgressBar(batteryBar);
}

// ******** CLICK **********//

static void SelectSingleClickHandler(ClickRecognizerRef recognizer, Window *window)
{
	if(GlobalState_GetCurrent() == STATE_DIALOG)
	{
		GlobalState_Pop();
		return;
	}
	if(GlobalState_GetCurrent() == STATE_LARGE_IMAGE)
	{
		GlobalState_Pop();
		return;
	}
	if(IsMenuUsable(GetMainMenu()))
	{
		CallMenuSelectCallback(GetMainMenu(), recognizer, window);
		if(hideMenuOnSelect)
		{
			HideMenu(GetMainMenu());
			if(IsMenuUsable(GetSlaveMenu()))
				HideMenu(GetSlaveMenu());
		}
	}
	else if(IsMenuHidden(GetMainMenu()))
	{
		if(GetMenuTotalCellCount(GetMainMenu()) > 0)
		{
			TriggerMenu(GetMainMenu());
			if(useSlaveMenu)
			{
				ShowMenu(GetSlaveMenu());
			}
		}
	}
}

static void UpSingleClickHandler(ClickRecognizerRef recognizer, Window *window)
{
	if(IsMenuUsable(GetMainMenu()))
	{
		MenuRowAlign align = MenuRowAlignCenter;
		menu_layer_set_selected_next(GetMenuLayer(GetMainMenu()), true, align, true);
		if(IsMenuUsable(GetSlaveMenu()))
		{
			menu_layer_set_selected_next(GetMenuLayer(GetSlaveMenu()), true, align, true);
		}
	}
}

static void DownSingleClickHandler(ClickRecognizerRef recognizer, Window *window)
{
	if(IsMenuUsable(GetMainMenu()))
	{
		MenuRowAlign align = MenuRowAlignCenter;
		menu_layer_set_selected_next(GetMenuLayer(GetMainMenu()), false, align, true);
		if(IsMenuUsable(GetSlaveMenu()))
		{
			menu_layer_set_selected_next(GetMenuLayer(GetSlaveMenu()), false, align, true);
		}
	}
}

static DialogData exitPrompt =
{
    .text = "Are you sure you want to exit the game?",
    .allowCancel = true
};

static void BackSingleClickHandler(ClickRecognizerRef recognizer, Window *window)
{
	switch(GlobalState_GetCurrent())
	{
		case STATE_MENU:
		{
			if(IsMenuUsable(GetMainMenu()))
			{
				HideMenu(GetMainMenu());
				if(IsMenuUsable(GetSlaveMenu()))
					HideMenu(GetSlaveMenu());
				return;
			}
			break;
		}
		case STATE_BATTLE:
		{
			SaveBattleState();
			break;
		}
        case STATE_ADVENTURE:
        {
            TriggerDialog(&exitPrompt);
            GlobalState_QueueStatePop();
            break;
        }
        case STATE_DIALOG:
        {
            if(Dialog_AllowCancel())
            {
                GlobalState_ClearQueue();
                GlobalState_Pop();
            }
            else
            {
                GlobalState_Pop();
            }
            break;
        }
		default:
		{
			GlobalState_Pop();
			break;
		}
	}
}

static void MenuClickConfigProvider(void *context)
{
	window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler)SelectSingleClickHandler);
	window_single_click_subscribe(BUTTON_ID_UP,(ClickHandler)UpSingleClickHandler);
	window_single_click_subscribe(BUTTON_ID_DOWN,(ClickHandler)DownSingleClickHandler);

	window_single_click_subscribe(BUTTON_ID_BACK, (ClickHandler)BackSingleClickHandler);
}

//******* WINDOW *********//

void BaseWindowAppear(Window *window)
{
	DEBUG_LOG("BaseWindowAppear");
	InitializeDescriptionLayer(window);
	InitializeMainImageLayer(window);
	InitializeClockLayer(window);
	InitializeProgressBar(batteryBar, window);
	UpdateBatteryLevel(battery_state_service_peek());
	InitializeMenuLayer(GetMainMenu(), window);
	InitializeMenuLayer(GetSlaveMenu(), window);
	InitializeMenuArrowLayer(window);
	InitializeDialogLayer(window);
	DEBUG_LOG("BaseWindowAppear end");
}

void BaseWindowDisappear(Window *window)
{
	RemoveClockLayer();
	RemoveMenuArrowLayer();
	RemoveMainImageLayer();
	RemoveDescriptionLayer();
	RemoveDialogLayer();
	RemoveProgressBar(batteryBar);
}

void BaseWindowUnload(Window *window)
{
	FreeMenuArrowLayer();
	FreeClockLayer();
	FreeDescriptionLayer();
	CleanupMainImageLayer();
	CleanupMenu(mainMenu);
	CleanupMenu(slaveMenu);
	FreeDialogLayer();
	FreeProgressBar(batteryBar);
}

void SetWindowHandlers(Window *window)
{
	WindowHandlers handlers = {.load = NULL, .unload = BaseWindowUnload, .appear = BaseWindowAppear, .disappear = BaseWindowDisappear};
	window_set_window_handlers(window,handlers);
}

Window * InitializeBaseWindow(void)
{
	INFO_LOG("InitializeBaseWindow");
	Window *window = window_create();
	window_set_background_color(window, GColorBlack);
	SetWindowHandlers(window);
	slaveMenu = CreateMenuLayer(15,
								48,
								80,
								84,
								4,
								false,
								false);
	mainMenu = CreateMenuLayer(95,
							   48,
							   75,
							   84,
							   4,
							   true,
							   true);
    GRect batteryFrame = BATTERY_FRAME;
	batteryBar = CreateProgressBar(&currentBatteryLevel, &maxBatteryLevel, FILL_UP, &batteryFrame, GColorBrightGreen, -1);
	window_set_click_config_provider(window, MenuClickConfigProvider);
	return window;		
}

