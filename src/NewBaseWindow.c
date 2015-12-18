#include <pebble.h>
#include "Clock.h"
#include "DescriptionFrame.h"
#include "DialogFrame.h"
#include "GlobalState.h"
#include "MainImage.h"
#include "MenuArrow.h"
#include "MiniAdventure.h"
#include "NewBaseWindow.h"
#include "NewBattle.h"
#include "NewMenu.h"
#include "Logging.h"
#include "Utils.h"

static bool usingNewWindow = false;

static Menu *mainMenu = NULL;
static Menu *slaveMenu = NULL;
static bool useSlaveMenu = false;
static bool hideMenuOnSelect = true;
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

bool UsingNewWindow(void)
{
	return usingNewWindow;
}

// ******** CLICK **********//

static void SelectSingleClickHandler(ClickRecognizerRef recognizer, Window *window)
{
	if(GetCurrentGlobalState() == STATE_DIALOG)
	{
		PopGlobalState();
		return;
	}
	if(IsMenuUsable(GetMainMenu()))
	{
		CallNewMenuSelectCallback(GetMainMenu(), recognizer, window);
		if(hideMenuOnSelect)
		{
			HideMenu(GetMainMenu()); //TODO: When implementing the options menu, I won't want this behavior, but I should be able to push a new click handler.
			if(IsMenuUsable(GetSlaveMenu()))
				HideMenu(GetSlaveMenu());
		}
	}
	else if(IsMenuHidden(GetMainMenu()))
	{
		if(GetMenuCellCount(GetMainMenu()) > 0)
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
		menu_layer_set_selected_next(GetNewMenuLayer(GetMainMenu()), true, MenuRowAlignCenter, true);
		if(IsMenuUsable(GetSlaveMenu()))
		{
			menu_layer_set_selected_next(GetNewMenuLayer(GetSlaveMenu()), true, MenuRowAlignCenter, true);
		}
	}
}

static void DownSingleClickHandler(ClickRecognizerRef recognizer, Window *window)
{
	if(IsMenuUsable(GetMainMenu()))
	{
		menu_layer_set_selected_next(GetNewMenuLayer(GetMainMenu()), false, MenuRowAlignCenter, true);
		if(IsMenuUsable(GetSlaveMenu()))
		{
			menu_layer_set_selected_next(GetNewMenuLayer(GetSlaveMenu()), false, MenuRowAlignCenter, true);
		}
	}
}

static void BackSingleClickHandler(ClickRecognizerRef recognizer, Window *window)
{
	switch(GetCurrentGlobalState())
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
		default:
		{
			PopGlobalState();
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
	InitializeNewClockLayer(window);
	InitializeNewMenuLayer(GetMainMenu(), window);
	InitializeNewMenuLayer(GetSlaveMenu(), window);
	InitializeMenuArrowLayer(window);
	InitializeDialogLayer(window);
	DEBUG_LOG("BaseWindowAppear end");
}

void BaseWindowDisappear(Window *window)
{
	RemoveNewClockLayer();
	RemoveMenuArrowLayer();
	RemoveMainImageLayer();
	RemoveDescriptionLayer();
	RemoveDialogLayer();
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
}

void SetWindowHandlers(Window *window)
{
	WindowHandlers handlers = {.load = NULL, .unload = BaseWindowUnload, .appear = BaseWindowAppear, .disappear = BaseWindowDisappear};
	window_set_window_handlers(window,handlers);
}

Window * InitializeNewBaseWindow(void)
{
	INFO_LOG("InitializeNewBaseWindow");
	Window *window = window_create();
	usingNewWindow = true;
	window_set_background_color(window, GColorBlack);
	SetWindowHandlers(window);
	slaveMenu = CreateMenuLayer(10,
								50,
								100,
								80,
								4,
								false,
								false);
	mainMenu = CreateMenuLayer(110,
							   50,
							   80,
							   80,
							   4,
							   true,
							   true);
	window_set_click_config_provider(window, MenuClickConfigProvider);
	return window;		
}

