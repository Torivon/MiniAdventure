#include <pebble.h>
#include "NewMenu.h"
#include "NewBaseWindow.h"

#define WINDOW_ROW_HEIGHT 16
#define NUM_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS 10

#define MENU_LAYER_X 50
#define MENU_LAYER_Y 20
#define MENU_LAYER_W 80
#define MENU_LAYER_H 100

#define MENU_ANIMATION_DURATION 500

static GFont menuFont;
static GRect newMenuOnScreenPosition = {.origin = {.x = MENU_LAYER_X, .y = MENU_LAYER_Y}, .size = {.w = MENU_LAYER_W, .h = MENU_LAYER_H}};
static GRect newMenuOffScreenPosition = {.origin = {.x = PBL_IF_RECT_ELSE(144, 180), .y = MENU_LAYER_Y}, .size = {.w = MENU_LAYER_W, .h = MENU_LAYER_H}};
static PropertyAnimation *menuShowAnimation = NULL;
static PropertyAnimation *menuHideAnimation = NULL;

static bool newMenuLayerInitialized = false;
static Layer *newTopLevelMenuLayer = NULL;
static MenuLayer *newMenuLayer = NULL;

static bool menuVisible = false;
static bool menuAnimating = false;

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
	SetMenuArrowLeft();
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
	SetMenuArrowRight();
}

static void HideAnimationStopped(struct Animation *animation, bool finished, void *context)
{
	menuAnimating = false;
	
	if(finished)
		menuVisible = false;
	
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
      return NUM_FIRST_MENU_ITEMS;
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
// Determine which section we're going to draw in
	char buffer[20];
	snprintf(buffer, sizeof(buffer), "Section %d, item %d", cell_index->section, cell_index->row);
	GRect bounds = layer_get_bounds(cell_layer);
	graphics_draw_text(ctx, buffer, menuFont, bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
//	menu_cell_basic_draw(ctx, cell_layer, NULL, buffer, NULL);
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) 
{
}

void InitializeNewMenuLayer(Window *window)
{
	if(!newMenuLayerInitialized)
	{		
		menuFont = fonts_get_system_font(FONT_KEY_GOTHIC_14);
		newTopLevelMenuLayer = layer_create(newMenuOffScreenPosition);
		newMenuLayer = menu_layer_create(layer_get_bounds(newTopLevelMenuLayer));
		layer_add_child(newTopLevelMenuLayer, menu_layer_get_layer(newMenuLayer));
		menu_layer_set_callbacks(newMenuLayer, NULL, (MenuLayerCallbacks){
			.get_num_sections = menu_get_num_sections_callback,
			.get_num_rows = menu_get_num_rows_callback,
			.draw_row = menu_draw_row_callback,
			.select_click = menu_select_callback,
			.get_cell_height = get_cell_height_callback,
		});
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
		newMenuLayerInitialized = false;
		if(menuShowAnimation)
			property_animation_destroy(menuShowAnimation);
		if(menuHideAnimation)
			property_animation_destroy(menuHideAnimation);
	}
}
