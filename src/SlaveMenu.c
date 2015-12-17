#include <pebble.h>

#if 0

#include "DescriptionFrame.h"
#include "GlobalState.h"
#include "Logging.h"
#include "MenuArrow.h"
#include "NewMenu.h"
#include "SlaveMenu.h"
#include "NewBaseWindow.h"

#define WINDOW_ROW_HEIGHT 16
#define NUM_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS 10

#define MENU_LAYER_X 20
#define MENU_LAYER_Y 20
#define MENU_LAYER_W 100
#define MENU_LAYER_H 100

#define INTERNAL_MENU_OFFSET 4

static GBitmap *slaveMenuBackgroundFrame = NULL;
static BitmapLayer *slaveMenuBackgroundLayer = NULL;

static GFont menuFont;
static GRect newMenuOnScreenPosition = {.origin = {.x = MENU_LAYER_X, .y = MENU_LAYER_Y}, .size = {.w = MENU_LAYER_W, .h = MENU_LAYER_H}};
static GRect newMenuOffScreenPosition = {.origin = {.x = 0 - MENU_LAYER_W, .y = MENU_LAYER_Y}, .size = {.w = MENU_LAYER_W, .h = MENU_LAYER_H}};
static PropertyAnimation *slaveMenuShowAnimation = NULL;
static PropertyAnimation *slaveMenuHideAnimation = NULL;

static bool slaveMenuLayerInitialized = false;
static Layer *slaveTopLevelMenuLayer = NULL;
static MenuLayer *slaveMenuLayer = NULL;

static bool slaveMenuVisible = false;
static bool slaveMenuAnimating = false;


static bool useCallbackFunctions = false;
static uint16_t cellCount = 0;
static MenuCellDescription *cellList = NULL;
MenuCountCallback slaveMenuCountCallback = NULL;
MenuNameCallback slaveMenuNameCallback = NULL;
MenuDescriptionCallback slaveMenuDescriptionCallback = NULL;
MenuSelectCallback slaveMenuSelectCallback = NULL;

uint16_t GetSlaveMenuCellCount(void)
{
	if(useCallbackFunctions)
	{
		return menuCountCallback();
	}
	else
	{
		return cellCount;
	}
}

const char *GetSlaveMenuName(MenuIndex *index)
{
	if(useCallbackFunctions)
	{
		return slaveMenuNameCallback(index->row);
	}
	else
	{
		return cellList[index->row].name;
	}
}

const char *GetMenuDescription(MenuIndex *index)
{
	if(useCallbackFunctions)
	{
		return menuDescriptionCallback(index->row);
	}
	else
	{
		return cellList[index->row].description;
	}
}

void CallNewMenuSelectCallback(ClickRecognizerRef recognizer, Window *window)
{
	if(!newMenuLayerInitialized)
		return;
	
	MenuIndex index = menu_layer_get_selected_index(newMenuLayer);
	
	if(index.row < GetMenuCellCount())
	{
		if(useCallbackFunctions)
		{
			menuSelectCallback(index.row);
		}
		else
		{
			cellList[index.row].callback();
		}
	}
}

void RegisterMenuCellList(MenuCellDescription *list, uint16_t count)
{
	DEBUG_LOG("RegisterMenuCellList");
	if(count == 0)
	{
		ClearMenuCellList();
		return;
	}
	
	useCallbackFunctions = false;
	menuCountCallback = NULL;
	menuNameCallback = NULL;
	menuDescriptionCallback = NULL;
	menuSelectCallback = NULL;
	
	cellList = list;
	cellCount = count;
	if(newMenuLayerInitialized)
	{
		menu_layer_reload_data(newMenuLayer);
		MenuIndex index = {.section = 0, .row = 0};
		menu_layer_set_selected_index(newMenuLayer, index, MenuRowAlignTop, false);
	}
	ShowMenuArrow();
}

void RegisterMenuCellCallbacks(MenuCountCallback countCallback, MenuNameCallback nameCallback, MenuDescriptionCallback descriptionCallback, MenuSelectCallback selectCallback)
{
	DEBUG_LOG("RegisterMenuCellCallbacks");
	useCallbackFunctions = true;
	menuCountCallback = countCallback;
	menuNameCallback = nameCallback;
	menuDescriptionCallback = descriptionCallback;
	menuSelectCallback = selectCallback;
	
	if(!menuCountCallback)
	{
		ClearMenuCellList();
		return;
	}
	
	cellCount = 0;
	cellList = NULL;

	if(menuCountCallback() > 0)
		ShowMenuArrow();
	else
		HideMenuArrow();
}

void ClearMenuCellList(void)
{
	cellCount = 0;
	cellList = NULL;
	useCallbackFunctions = false;
	menuCountCallback = NULL;
	menuNameCallback = NULL;
	menuDescriptionCallback = NULL;
	menuSelectCallback = NULL;
	
	HideMenuArrow();
}

MenuLayer *GetNewMenuLayer(void)
{
	return newMenuLayer;
}

bool IsMenuUsable(void)
{
	return menuVisible && !menuAnimating;
}

bool IsMenuHidden(void)
{
	return !menuVisible  && !menuAnimating;
}

bool IsMenuVisible(void)
{
	return menuVisible;
}

static void ShowAnimationStarted(struct Animation *animation, void *context)
{
	menuAnimating = true;
	ActivateMenuArrow();
	MenuIndex index = menu_layer_get_selected_index(newMenuLayer);
	
	if(index.row < GetMenuCellCount())
	{
		const char *newDescription = GetMenuDescription(&index);
		SetDescription(newDescription ? newDescription : "");
	}
}

static void ShowAnimationStopped(struct Animation *animation, bool finished, void *context)
{
	menuAnimating = false;
	
	if(finished)
		menuVisible = true;

#if !defined(PBL_PLATFORM_APLITE)
	menuShowAnimation = NULL;
#endif
}

static void HideAnimationStarted(struct Animation *animation, void *context)
{
	menuAnimating = true;
	InactivateMenuArrow();
}

static void HideAnimationStopped(struct Animation *animation, bool finished, void *context)
{
	menuAnimating = false;
	
	if(finished)
	{
		menuVisible = false;
		PopGlobalState();
	}
	
#if !defined(PBL_PLATFORM_APLITE)
	menuHideAnimation = NULL;	
#endif
}
	
void ShowMenu(void)
{
	if(menuShowAnimation && animation_is_scheduled((Animation*)menuShowAnimation))
		return;

	if(!menuShowAnimation)
	{
		menuShowAnimation = property_animation_create_layer_frame(newTopLevelMenuLayer, NULL, &newMenuOnScreenPosition);
		animation_set_duration((Animation*)menuShowAnimation, MENU_ANIMATION_DURATION);
		animation_set_curve((Animation*)menuShowAnimation, AnimationCurveLinear);
		AnimationHandlers showHandlers = {.started = ShowAnimationStarted, .stopped = ShowAnimationStopped};
		animation_set_handlers((Animation*)menuShowAnimation, showHandlers, NULL);
	}
	
	if(menuHideAnimation)
		animation_unschedule((Animation*) menuHideAnimation);
	animation_schedule((Animation*) menuShowAnimation);
}

void HideMenu(void)
{
	if(menuHideAnimation && animation_is_scheduled((Animation*)menuHideAnimation))
		return;

	if(!menuHideAnimation)
	{
		menuHideAnimation = property_animation_create_layer_frame(newTopLevelMenuLayer, NULL, &newMenuOffScreenPosition);
		animation_set_duration((Animation*)menuHideAnimation, MENU_ANIMATION_DURATION);
		animation_set_curve((Animation*)menuShowAnimation, AnimationCurveLinear);
		AnimationHandlers hideHandlers = {.started = HideAnimationStarted, .stopped = HideAnimationStopped};
		animation_set_handlers((Animation*)menuHideAnimation, hideHandlers, NULL);
	}
	
	if(menuShowAnimation)
		animation_unschedule((Animation*) menuShowAnimation);
	animation_schedule((Animation*) menuHideAnimation);
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) 
{
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) 
{
  switch (section_index) {
    case 0:
      return GetMenuCellCount();
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
	char buffer[20];
	snprintf(buffer, sizeof(buffer), "%s", GetMenuName(cell_index));
	GRect bounds = layer_get_bounds(cell_layer);
	graphics_draw_text(ctx, buffer, menuFont, bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

void selection_changed_callback(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context)
{
	const char *newDescription = GetMenuDescription(&new_index);
	SetDescription(newDescription ? newDescription : "");
}

void InitializeNewMenuLayer(Window *window)
{
	if(!newMenuLayerInitialized)
	{
		newMenuBackgroundFrame = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_FRAME);
		GRect frameBounds = gbitmap_get_bounds(newMenuBackgroundFrame);
		GRect windowBounds = layer_get_bounds(window_get_root_layer(window));
		newMenuOnScreenPosition.size = frameBounds.size;
		newMenuOffScreenPosition.size = frameBounds.size;
		newMenuOnScreenPosition.origin.y = windowBounds.size.h / 2 - newMenuOnScreenPosition.size.h / 2;
		newMenuOnScreenPosition.origin.x = windowBounds.size.w / 2 - newMenuOnScreenPosition.size.w / 2;
		newMenuOffScreenPosition.origin.y = windowBounds.size.h / 2 - newMenuOnScreenPosition.size.h / 2;
		menuFont = fonts_get_system_font(FONT_KEY_GOTHIC_14);
		newTopLevelMenuLayer = layer_create(newMenuOffScreenPosition);
		newMenuBackgroundLayer = bitmap_layer_create(layer_get_bounds(newTopLevelMenuLayer));
		bitmap_layer_set_bitmap(newMenuBackgroundLayer, newMenuBackgroundFrame);
		bitmap_layer_set_alignment(newMenuBackgroundLayer, GAlignCenter);
		layer_add_child(newTopLevelMenuLayer, bitmap_layer_get_layer(newMenuBackgroundLayer));
		GRect menu_bounds = layer_get_bounds(newTopLevelMenuLayer);
		menu_bounds.origin.x += INTERNAL_MENU_OFFSET;
		menu_bounds.origin.y += INTERNAL_MENU_OFFSET;
		menu_bounds.size.w -= 2 * INTERNAL_MENU_OFFSET;
		menu_bounds.size.h -= 2 * INTERNAL_MENU_OFFSET;
		newMenuLayer = menu_layer_create(menu_bounds);
		layer_add_child(newTopLevelMenuLayer, menu_layer_get_layer(newMenuLayer));
		menu_layer_set_callbacks(newMenuLayer, NULL, (MenuLayerCallbacks){
			.get_num_sections = menu_get_num_sections_callback,
			.get_num_rows = menu_get_num_rows_callback,
			.draw_row = menu_draw_row_callback,
			.get_cell_height = get_cell_height_callback,
			.selection_changed = selection_changed_callback
		});
#if defined(PBL_COLOR)
		menu_layer_set_normal_colors(newMenuLayer, GColorBlue, GColorWhite);
		menu_layer_set_highlight_colors(newMenuLayer, GColorWhite, GColorBlue);
#endif
		scroll_layer_set_shadow_hidden(menu_layer_get_scroll_layer(newMenuLayer), false);

		newMenuLayerInitialized = true;
	}
	Layer *window_layer = window_get_root_layer(window);
	layer_add_child(window_layer, newTopLevelMenuLayer);
}

void RemoveNewMenuLayer(Window *window)
{
	if(!newMenuLayerInitialized)
		return;
	
	layer_remove_from_parent(newTopLevelMenuLayer);
}

void CleanupMenu(void)
{
	if(newMenuLayerInitialized)
	{
		layer_destroy(newTopLevelMenuLayer);
		menu_layer_destroy(newMenuLayer);
		bitmap_layer_destroy(newMenuBackgroundLayer);
		gbitmap_destroy(newMenuBackgroundFrame);
		newMenuLayerInitialized = false;
		if(menuShowAnimation)
			property_animation_destroy(menuShowAnimation);
		if(menuHideAnimation)
			property_animation_destroy(menuHideAnimation);
	}
}

void ReloadMenu(void)
{
	DEBUG_LOG("ReloadMenu");
	if(newMenuLayerInitialized)
	{
		DEBUG_LOG("Reloading");
		menu_layer_reload_data(newMenuLayer);
		DEBUG_LOG("%d menu cells", GetMenuCellCount());
		if(GetMenuCellCount() > 0)
		{
			ShowMenuArrow();
		}
		else
		{
			HideMenuArrow();
		}
	}
}

void TriggerMenu(void)
{
	PushGlobalState(MENU, 0, NULL, ShowMenu, NULL, NULL, NULL);
}

#endif