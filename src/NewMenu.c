#include <pebble.h>
#include "DescriptionFrame.h"
#include "GlobalState.h"
#include "Logging.h"
#include "MenuArrow.h"
#include "NewMenu.h"
#include "NewBaseWindow.h"
#include "Utils.h"

#define WINDOW_ROW_HEIGHT 16
#define NUM_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS 10

typedef struct Menu
{
	int onScreenX;
	int yPos;
	int width;
	int height;
	int innerOffset;
	bool offScreenRight;
	bool mainMenu;
	
	GFont menuFont;
	GRect newMenuOnScreenPosition;
	GRect newMenuOffScreenPosition;
	PropertyAnimation *menuShowAnimation;
	PropertyAnimation *menuHideAnimation;

	bool newMenuLayerInitialized;
	Layer *newTopLevelMenuLayer;
	MenuLayer *newMenuLayer;

	bool menuVisible;
	bool menuAnimating;


	bool useCallbackFunctions;
	uint16_t cellCount;
	MenuCellDescription *cellList;
	MenuCountCallback menuCountCallback;
	MenuNameCallback menuNameCallback;
	MenuDescriptionCallback menuDescriptionCallback;
	MenuSelectCallback menuSelectCallback;
} Menu;

uint16_t GetMenuCellCount(Menu *menu)
{
	if(menu->useCallbackFunctions)
	{
		return menu->menuCountCallback();
	}
	else
	{
		return menu->cellCount;
	}
}

const char *GetMenuName(Menu *menu, MenuIndex *index)
{
	if(menu->useCallbackFunctions)
	{
		return menu->menuNameCallback(index->row);
	}
	else
	{
		return menu->cellList[index->row].name;
	}
}

const char *GetMenuDescription(Menu *menu, MenuIndex *index)
{
	if(menu->useCallbackFunctions)
	{
		return menu->menuDescriptionCallback(index->row);
	}
	else
	{
		return menu->cellList[index->row].description;
	}
}

void CallNewMenuSelectCallback(Menu *menu, ClickRecognizerRef recognizer, Window *window)
{
	if(!menu->newMenuLayerInitialized)
		return;
	
	MenuIndex index = menu_layer_get_selected_index(menu->newMenuLayer);
	
	if(index.row < GetMenuCellCount(menu))
	{
		if(menu->useCallbackFunctions)
		{
			menu->menuSelectCallback(index.row);
		}
		else
		{
			menu->cellList[index.row].callback();
		}
	}
}

void RegisterMenuCellList(Menu *menu, MenuCellDescription *list, uint16_t count)
{
	DEBUG_LOG("RegisterMenuCellList");
	if(count == 0)
	{
		ClearMenuCellList(menu);
		return;
	}
	
	menu->useCallbackFunctions = false;
	menu->menuCountCallback = NULL;
	menu->menuNameCallback = NULL;
	menu->menuDescriptionCallback = NULL;
	menu->menuSelectCallback = NULL;
	
	menu->cellList = list;
	menu->cellCount = count;
	if(menu->newMenuLayerInitialized)
	{
		menu_layer_reload_data(menu->newMenuLayer);
		MenuIndex index = {.section = 0, .row = 0};
		menu_layer_set_selected_index(menu->newMenuLayer, index, MenuRowAlignCenter, false);
	}
	if(menu->mainMenu)
		ShowMenuArrow();
}

void RegisterMenuCellCallbacks(Menu *menu, MenuCountCallback countCallback, MenuNameCallback nameCallback, MenuDescriptionCallback descriptionCallback, MenuSelectCallback selectCallback)
{
	DEBUG_LOG("RegisterMenuCellCallbacks");
	menu->useCallbackFunctions = true;
	menu->menuCountCallback = countCallback;
	menu->menuNameCallback = nameCallback;
	menu->menuDescriptionCallback = descriptionCallback;
	menu->menuSelectCallback = selectCallback;
	
	if(!menu->menuCountCallback)
	{
		ClearMenuCellList(menu);
		return;
	}
	
	menu->cellCount = 0;
	menu->cellList = NULL;

	if(menu->mainMenu)
	{
		if(menu->menuCountCallback() > 0)
			ShowMenuArrow();
		else
			HideMenuArrow();
	}
}

void ClearMenuCellList(Menu *menu)
{
	menu->cellCount = 0;
	menu->cellList = NULL;
	menu->useCallbackFunctions = false;
	menu->menuCountCallback = NULL;
	menu->menuNameCallback = NULL;
	menu->menuDescriptionCallback = NULL;
	menu->menuSelectCallback = NULL;
	
	if(menu->mainMenu)
		HideMenuArrow();
}

MenuLayer *GetNewMenuLayer(Menu *menu)
{
	return menu->newMenuLayer;
}

bool IsMenuUsable(Menu *menu)
{
	return menu->menuVisible && !menu->menuAnimating;
}

bool IsMenuHidden(Menu *menu)
{
	return !menu->menuVisible  && !menu->menuAnimating;
}

bool IsMenuVisible(Menu *menu)
{
	return menu->menuVisible;
}

static void ShowAnimationStarted(struct Animation *animation, void *context)
{
	Menu *menu = (Menu*)context;
	menu->menuAnimating = true;
	if(menu->mainMenu)
		ActivateMenuArrow();
	MenuIndex index = menu_layer_get_selected_index(menu->newMenuLayer);
	
	if(index.row < GetMenuCellCount(menu))
	{
		const char *newDescription = GetMenuDescription(menu, &index);
		if(menu->mainMenu)
			SetDescription(newDescription ? newDescription : "");
	}
}

static void ShowAnimationStopped(struct Animation *animation, bool finished, void *context)
{
	Menu *menu = (Menu*)context;
	menu->menuAnimating = false;
	
	if(finished)
		menu->menuVisible = true;

	menu->menuShowAnimation = NULL;
}

static void HideAnimationStarted(struct Animation *animation, void *context)
{
	Menu *menu = (Menu*)context;
	menu->menuAnimating = true;
	if(menu->mainMenu)
		InactivateMenuArrow();
}

static void HideAnimationStopped(struct Animation *animation, bool finished, void *context)
{
	Menu *menu = (Menu*)context;
	menu->menuAnimating = false;
	
	if(finished)
	{
		menu->menuVisible = false;
		if(menu->mainMenu)
			PopGlobalState();
	}
	
	menu->menuHideAnimation = NULL;	
}
	
void ShowMenu(void *data)
{
	Menu *menu = (Menu*)data;
	if(menu->menuShowAnimation && animation_is_scheduled((Animation*)menu->menuShowAnimation))
		return;

	if(!menu->menuShowAnimation)
	{
		menu->menuShowAnimation = property_animation_create_layer_frame(menu->newTopLevelMenuLayer, NULL, &menu->newMenuOnScreenPosition);
		animation_set_duration((Animation*)menu->menuShowAnimation, MENU_ANIMATION_DURATION);
		animation_set_curve((Animation*)menu->menuShowAnimation, AnimationCurveLinear);
		AnimationHandlers showHandlers = {.started = ShowAnimationStarted, .stopped = ShowAnimationStopped};
		animation_set_handlers((Animation*)menu->menuShowAnimation, showHandlers, menu);
	}
	
	if(menu->menuHideAnimation)
		animation_unschedule((Animation*) menu->menuHideAnimation);
	animation_schedule((Animation*) menu->menuShowAnimation);
}

void HideMenu(Menu *menu)
{
	if(menu->menuHideAnimation && animation_is_scheduled((Animation*)menu->menuHideAnimation))
		return;

	if(!menu->menuHideAnimation)
	{
		menu->menuHideAnimation = property_animation_create_layer_frame(menu->newTopLevelMenuLayer, NULL, &menu->newMenuOffScreenPosition);
		animation_set_duration((Animation*)menu->menuHideAnimation, MENU_ANIMATION_DURATION);
		animation_set_curve((Animation*)menu->menuShowAnimation, AnimationCurveLinear);
		AnimationHandlers hideHandlers = {.started = HideAnimationStarted, .stopped = HideAnimationStopped};
		animation_set_handlers((Animation*)menu->menuHideAnimation, hideHandlers, menu);
	}
	
	if(menu->menuShowAnimation)
		animation_unschedule((Animation*) menu->menuShowAnimation);
	animation_schedule((Animation*) menu->menuHideAnimation);
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) 
{
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) 
{
	Menu *menu = (Menu*)data;
	switch (section_index) {
		case 0:
		return GetMenuCellCount(menu);
		default:
		return 0;
  }
}

int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context)
{
	return WINDOW_ROW_HEIGHT;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) 
{
	Menu *menu = (Menu*)data;
	char buffer[20];
	snprintf(buffer, sizeof(buffer), "%s", GetMenuName(menu, cell_index));
	GRect bounds = layer_get_bounds(cell_layer);
	graphics_draw_text(ctx, buffer, menu->menuFont, bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

void selection_changed_callback(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context)
{
	Menu *menu = (Menu*)callback_context;
	const char *newDescription = GetMenuDescription(menu, &new_index);
	if(menu->mainMenu)
		SetDescription(newDescription ? newDescription : "");
}

Menu *CreateMenuLayer(int onScreenX,
					  int yPos,
					  int width,
					  int height,
					  int innerOffset,
					  bool offScreenRight,
					  bool mainMenu)
{
	Menu *menu = calloc(sizeof(Menu), 1);

	menu->onScreenX = onScreenX;
	menu->yPos = yPos;
	menu->width = width;
	menu->height = height;
	menu->innerOffset = innerOffset;
	menu->offScreenRight = offScreenRight;
	menu->mainMenu = mainMenu;
	return menu;
}

void MenuUpdateProc(struct Layer *layer, GContext *ctx)
{
	GRect bounds = layer_get_bounds(layer);
	DrawContentFrame(ctx, &bounds);
}

void InitializeNewMenuLayer(Menu *menu, Window *window)
{
	if(!menu->newMenuLayerInitialized)
	{
		GRect windowBounds = layer_get_bounds(window_get_root_layer(window));
		menu->newMenuOnScreenPosition.size.w = menu->width;
		menu->newMenuOnScreenPosition.size.h = menu->height;
		menu->newMenuOffScreenPosition.size = menu->newMenuOnScreenPosition.size;
		menu->newMenuOnScreenPosition.origin.x = menu->onScreenX;
		menu->newMenuOnScreenPosition.origin.y = menu->yPos;
		menu->newMenuOffScreenPosition.origin.y = menu->yPos;
		if(menu->offScreenRight)
		{
			menu->newMenuOffScreenPosition.origin.x = windowBounds.size.w;
		}
		else
		{
			menu->newMenuOffScreenPosition.origin.x = -menu->newMenuOffScreenPosition.size.w;
		}
		menu->menuFont = fonts_get_system_font(FONT_KEY_GOTHIC_14);
		menu->newTopLevelMenuLayer = layer_create(menu->newMenuOffScreenPosition);
		GRect menu_bounds = layer_get_bounds(menu->newTopLevelMenuLayer);
		menu_bounds.origin.x += menu->innerOffset;
		menu_bounds.origin.y += menu->innerOffset;
		menu_bounds.size.w -= 2 * menu->innerOffset;
		menu_bounds.size.h -= 2 * menu->innerOffset;
		menu->newMenuLayer = menu_layer_create(menu_bounds);
		layer_add_child(menu->newTopLevelMenuLayer, menu_layer_get_layer(menu->newMenuLayer));
		menu_layer_set_callbacks(menu->newMenuLayer, menu, (MenuLayerCallbacks){
			.get_num_sections = menu_get_num_sections_callback,
			.get_num_rows = menu_get_num_rows_callback,
			.draw_row = menu_draw_row_callback,
			.get_cell_height = get_cell_height_callback,
			.selection_changed = selection_changed_callback
		});
#if defined(PBL_COLOR)
		menu_layer_set_normal_colors(menu->newMenuLayer, GColorBlue, GColorWhite);
		menu_layer_set_highlight_colors(menu->newMenuLayer, GColorWhite, GColorBlue);
#endif
		scroll_layer_set_shadow_hidden(menu_layer_get_scroll_layer(menu->newMenuLayer), false);

		layer_set_update_proc(menu->newTopLevelMenuLayer, MenuUpdateProc);
		menu->newMenuLayerInitialized = true;
	}
	Layer *window_layer = window_get_root_layer(window);
	layer_add_child(window_layer, menu->newTopLevelMenuLayer);
}

void RemoveNewMenuLayer(Menu *menu, Window *window)
{
	if(!menu->newMenuLayerInitialized)
		return;
	
	layer_remove_from_parent(menu->newTopLevelMenuLayer);
}

void CleanupMenu(Menu *menu)
{
	if(!menu)
		return;
	
	if(menu->newMenuLayerInitialized)
	{
		layer_destroy(menu->newTopLevelMenuLayer);
		menu_layer_destroy(menu->newMenuLayer);
		menu->newMenuLayerInitialized = false;
		if(menu->menuShowAnimation)
			property_animation_destroy(menu->menuShowAnimation);
		if(menu->menuHideAnimation)
			property_animation_destroy(menu->menuHideAnimation);
	}
	
	free(menu);
}

void ReloadMenu(Menu *menu)
{
	DEBUG_LOG("ReloadMenu");
	if(menu->newMenuLayerInitialized)
	{
		DEBUG_LOG("Reloading");
		menu_layer_reload_data(menu->newMenuLayer);
		DEBUG_LOG("%d menu cells", GetMenuCellCount(menu));
		if(menu->mainMenu)
		{
			if(GetMenuCellCount(menu) > 0)
			{
				ShowMenuArrow();
			}
			else
			{
				HideMenuArrow();
			}
		}
	}
}

void TriggerMenu(Menu *menu)
{
	PushGlobalState(STATE_MENU, 0, NULL, ShowMenu, NULL, NULL, NULL, menu);
}
