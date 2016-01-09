#include <pebble.h>
#include "DescriptionFrame.h"
#include "GlobalState.h"
#include "Logging.h"
#include "MenuArrow.h"
#include "Menu.h"
#include "BaseWindow.h"
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
    GFont headerFont;
	GRect menuOnScreenPosition;
	GRect menuOffScreenPosition;
	PropertyAnimation *menuShowAnimation;
	PropertyAnimation *menuHideAnimation;

	bool menuLayerInitialized;
	Layer *topLevelMenuLayer;
	MenuLayer *menuLayer;

	bool menuVisible;
	bool menuAnimating;

    MenuSectionCountCallback menuSectionCountCallback;
    MenuSectionNameCallback menuSectionNameCallback;
	MenuCountCallback menuCountCallback;
	MenuNameCallback menuNameCallback;
	MenuDescriptionCallback menuDescriptionCallback;
	MenuSelectCallback menuSelectCallback;
} Menu;

uint16_t GetMenuCellCount(Menu *menu, uint16_t section_index)
{
    if(menu->menuCountCallback)
        return menu->menuCountCallback(section_index);
    else
        return 0;
}

uint16_t GetMenuSectionCount(Menu *menu)
{
    if(menu->menuSectionCountCallback)
        return menu->menuSectionCountCallback();
    else
        return 0;
}

uint16_t GetMenuTotalCellCount(Menu *menu)
{
    uint16_t totalCells = 0;
    for(int i = 0; i < GetMenuSectionCount(menu); ++i)
    {
        totalCells += GetMenuCellCount(menu, i);
    }
    return totalCells;
}

const char *GetMenuName(Menu *menu, MenuIndex *index)
{
    if(menu->menuNameCallback)
        return menu->menuNameCallback(index);
    else
        return "";
}

const char *GetMenuSectionName(Menu *menu, uint16_t section_index)
{
    if(menu->menuSectionNameCallback)
        return menu->menuSectionNameCallback(section_index);
    else
        return "";
}

const char *GetMenuDescription(Menu *menu, MenuIndex *index)
{
    if(menu->menuDescriptionCallback)
        return menu->menuDescriptionCallback(index);
    else
        return "";
}

void CallMenuSelectCallback(Menu *menu, ClickRecognizerRef recognizer, Window *window)
{
	if(!menu->menuLayerInitialized)
		return;
	
	MenuIndex index = menu_layer_get_selected_index(menu->menuLayer);
	
	if(index.row < GetMenuCellCount(menu, index.section))
	{
		menu->menuSelectCallback(&index);
	}
}

void RegisterMenuCellCallbacks(Menu *menu, MenuParameters *parameters)
{
	menu->menuCountCallback = parameters->countCallback;
	menu->menuNameCallback = parameters->nameCallback;
	menu->menuDescriptionCallback = parameters->descriptionCallback;
	menu->menuSelectCallback = parameters->selectCallback;
    menu->menuSectionCountCallback = parameters->menuSectionCountCallback;
    menu->menuSectionNameCallback = parameters->menuSectionNameCallback;
	
	if(!menu->menuCountCallback)
	{
		ClearMenuCellList(menu);
		return;
	}
	
    uint16_t totalCells = GetMenuTotalCellCount(menu);
    if(totalCells > 0)
    {
        menu_layer_reload_data(menu->menuLayer);
        MenuIndex index = {.section = 0, .row = 0};
        menu_layer_set_selected_index(menu->menuLayer, index, MenuRowAlignCenter, false);
        if(menu->mainMenu)
            ShowMenuArrow();
    }
    else if(menu->mainMenu)
    {
		HideMenuArrow();
	}
}

void ClearMenuCellList(Menu *menu)
{
	menu->menuCountCallback = NULL;
	menu->menuNameCallback = NULL;
	menu->menuDescriptionCallback = NULL;
	menu->menuSelectCallback = NULL;
    menu->menuSectionCountCallback = NULL;
    menu->menuSectionNameCallback = NULL;
	
	if(menu->mainMenu)
		HideMenuArrow();
}

MenuLayer *GetMenuLayer(Menu *menu)
{
	return menu->menuLayer;
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
	MenuIndex index = menu_layer_get_selected_index(menu->menuLayer);
	
	if(index.row < GetMenuCellCount(menu, index.section))
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
			GlobalState_Pop();
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
		menu->menuShowAnimation = property_animation_create_layer_frame(menu->topLevelMenuLayer, NULL, &menu->menuOnScreenPosition);
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
		menu->menuHideAnimation = property_animation_create_layer_frame(menu->topLevelMenuLayer, NULL, &menu->menuOffScreenPosition);
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
    Menu *menu = (Menu *) data;
    return GetMenuSectionCount(menu);
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) 
{
	Menu *menu = (Menu*)data;
    return GetMenuCellCount(menu, section_index);
}

int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context)
{
	return WINDOW_ROW_HEIGHT;
}

int16_t get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context)
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

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
    Menu *menu = (Menu*)data;
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%s", GetMenuSectionName(menu, section_index));
    GRect bounds = layer_get_bounds(cell_layer);
    graphics_draw_text(ctx, buffer, menu->headerFont, bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
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

void InitializeMenuLayer(Menu *menu, Window *window)
{
	if(!menu->menuLayerInitialized)
	{
		GRect windowBounds = layer_get_bounds(window_get_root_layer(window));
		menu->menuOnScreenPosition.size.w = menu->width;
		menu->menuOnScreenPosition.size.h = menu->height;
		menu->menuOffScreenPosition.size = menu->menuOnScreenPosition.size;
		menu->menuOnScreenPosition.origin.x = menu->onScreenX;
		menu->menuOnScreenPosition.origin.y = menu->yPos;
		menu->menuOffScreenPosition.origin.y = menu->yPos;
		if(menu->offScreenRight)
		{
			menu->menuOffScreenPosition.origin.x = windowBounds.size.w;
		}
		else
		{
			menu->menuOffScreenPosition.origin.x = -menu->menuOffScreenPosition.size.w;
		}
		menu->menuFont = fonts_get_system_font(FONT_KEY_GOTHIC_14);
        menu->headerFont = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
		menu->topLevelMenuLayer = layer_create(menu->menuOffScreenPosition);
		GRect menu_bounds = layer_get_bounds(menu->topLevelMenuLayer);
		menu_bounds.origin.x += menu->innerOffset;
		menu_bounds.origin.y += menu->innerOffset;
		menu_bounds.size.w -= 2 * menu->innerOffset;
		menu_bounds.size.h -= 2 * menu->innerOffset;
		menu->menuLayer = menu_layer_create(menu_bounds);
		layer_add_child(menu->topLevelMenuLayer, menu_layer_get_layer(menu->menuLayer));
		menu_layer_set_callbacks(menu->menuLayer, menu, (MenuLayerCallbacks){
			.get_num_sections = menu_get_num_sections_callback,
            .get_header_height = get_header_height_callback,
            .draw_header = menu_draw_header_callback,
			.get_num_rows = menu_get_num_rows_callback,
			.draw_row = menu_draw_row_callback,
			.get_cell_height = get_cell_height_callback,
			.selection_changed = selection_changed_callback
		});
#if defined(PBL_COLOR)
		menu_layer_set_normal_colors(menu->menuLayer, GColorBlue, GColorWhite);
		menu_layer_set_highlight_colors(menu->menuLayer, GColorWhite, GColorBlue);
#endif
		scroll_layer_set_shadow_hidden(menu_layer_get_scroll_layer(menu->menuLayer), false);

		layer_set_update_proc(menu->topLevelMenuLayer, MenuUpdateProc);
		menu->menuLayerInitialized = true;
	}
	Layer *window_layer = window_get_root_layer(window);
	layer_add_child(window_layer, menu->topLevelMenuLayer);
}

void RemoveMenuLayer(Menu *menu, Window *window)
{
	if(!menu->menuLayerInitialized)
		return;
	
	layer_remove_from_parent(menu->topLevelMenuLayer);
}

void CleanupMenu(Menu *menu)
{
	if(!menu)
		return;
	
	if(menu->menuLayerInitialized)
	{
		layer_destroy(menu->topLevelMenuLayer);
		menu_layer_destroy(menu->menuLayer);
		menu->menuLayerInitialized = false;
		if(menu->menuShowAnimation)
			property_animation_destroy(menu->menuShowAnimation);
		if(menu->menuHideAnimation)
			property_animation_destroy(menu->menuHideAnimation);
	}
	
	free(menu);
}

void ReloadMenu(Menu *menu)
{
	if(menu->menuLayerInitialized)
	{
		menu_layer_reload_data(menu->menuLayer);
		if(menu->mainMenu)
		{
			if(GetMenuTotalCellCount(menu) > 0)
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

void Menu_ResetSelection(Menu *menu)
{
    MenuIndex index = {.section = 0, .row = 0};
    menu_layer_set_selected_index(menu->menuLayer, index, MenuRowAlignCenter, false);
}

void TriggerMenu(Menu *menu)
{
	GlobalState_Push(STATE_MENU, 0, NULL, ShowMenu, NULL, NULL, NULL, menu);
}
