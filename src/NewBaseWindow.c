#include <pebble.h>
#include "Clock.h"
#include "MainImage.h"
#include "MenuArrow.h"
#include "MiniAdventure.h"
#include "NewBaseWindow.h"
#include "NewMenu.h"
#include "Logging.h"
#include "Utils.h"

static bool usingNewWindow = false;

bool UsingNewWindow(void)
{
	return usingNewWindow;
}

// ******** CLICK **********//

static void SelectSingleClickHandler(ClickRecognizerRef recognizer, Window *window)
{
	if(IsMenuUsable())
	{
		CallNewMenuSelectCallback(recognizer, window);
	}
	else
	{
		if(GetMenuCellCount() > 0)
			ShowMenu();
	}
}

static void UpSingleClickHandler(ClickRecognizerRef recognizer, Window *window)
{
	if(IsMenuUsable())
		menu_layer_set_selected_next(GetNewMenuLayer(), true, MenuRowAlignCenter, true);
}

static void DownSingleClickHandler(ClickRecognizerRef recognizer, Window *window)
{
	if(IsMenuUsable())
		menu_layer_set_selected_next(GetNewMenuLayer(), false, MenuRowAlignCenter, true);
}

static void BackSingleClickHandler(ClickRecognizerRef recognizer, Window *window)
{
	if(!IsMenuHidden())
	{
		HideMenu();
		return;
	}
		
	window_stack_pop(true);
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
	InitializeMainImageLayer(window);
	InitializeNewClockLayer(window);
	InitializeNewMenuLayer(window);
	InitializeMenuArrowLayer(window);
}

void BaseWindowDisappear(Window *window)
{
	RemoveNewClockLayer();
	RemoveMenuArrowLayer();
	RemoveMainImageLayer();
}

void BaseWindowUnload(Window *window)
{
	FreeMenuArrowLayer();
	FreeClockLayer();
	CleanupMainImageLayer();
}

void SetWindowHandlers(Window *window)
{
	WindowHandlers handlers = {.load = NULL, .unload = BaseWindowUnload, .appear = BaseWindowAppear, .disappear = BaseWindowDisappear};
	window_set_window_handlers(window,handlers);
}

Window * InitializeNewBaseWindow(void)
{
	Window *window = window_create();
	usingNewWindow = true;
#ifdef PBL_PLATFORM_APLITE
	window_set_fullscreen(window, true);
#endif
	window_set_background_color(window, GColorBlack);
	InitializeNewClockLayer(window);
	SetWindowHandlers(window);
	window_set_click_config_provider(window, MenuClickConfigProvider);
	return window;		
}

