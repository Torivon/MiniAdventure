#include <pebble.h>
#include "Adventure.h"
#include "BaseWindow.h"
#include "Battle.h"
#include "DescriptionFrame.h"
#include "EngineMenu.h"
#include "GlobalState.h"
#include "Logging.h"
#include "MenuArrow.h"
#include "Menu.h"
#include "OptionsMenu.h"
#include "TitleScreen.h"
#include "Utils.h"

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
    
    GlobalState menuState;
} Menu;

uint16_t GetMenuCellCount(Menu *menu, uint16_t section_index)
{
    GlobalState state = menu->menuState;
    if(menu->mainMenu)
    {
        switch(state)
        {
            case STATE_ENGINE_MENU:
            {
                return EngineMenu_GetCellCount();
                break;
            }
            case STATE_ADVENTURE:
            {
                return Adventure_MenuCellCount(section_index);
                break;
            }
            case STATE_OPTIONS:
            {
                return OptionsMenu_CellCount(section_index);
                break;
            }
            case STATE_BATTLE:
            {
                return BattleScreen_MenuCellCount(section_index);
                break;
            }
            case STATE_TITLE_SCREEN:
            {
                return TitleScreen_MenuCellCount(section_index);
                break;
            }
            default:
            {
                return 0;
            }
        }
    }
    else
    {
        switch(state)
        {
            case STATE_OPTIONS:
            {
                return OptionsMenu_CellCount(section_index);
                break;
            }
            default:
            {
                return 0;
            }
        }
    }
}

uint16_t GetMenuSectionCount(Menu *menu)
{
    GlobalState state = menu->menuState;
    if(menu->mainMenu)
    {
        switch(state)
        {
            case STATE_ENGINE_MENU:
            {
                return EngineMenu_GetSectionCount();
                break;
            }
            case STATE_ADVENTURE:
            {
                return Adventure_MenuSectionCount();
                break;
            }
            case STATE_OPTIONS:
            {
                return OptionsMenu_SectionCount();
                break;
            }
            case STATE_BATTLE:
            {
                return BattleScreen_MenuSectionCount();
                break;
            }
            case STATE_TITLE_SCREEN:
            {
                return TitleScreen_MenuSectionCount();
                break;
            }
            default:
            {
                return 0;
            }
        }
    }
    else
    {
        switch(state)
        {
            case STATE_OPTIONS:
            {
                return OptionsMenu_SectionCount();
                break;
            }
            default:
            {
                return 0;
            }
        }
    }
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
    GlobalState state = menu->menuState;
    switch(state)
    {
        case STATE_ENGINE_MENU:
        {
            return EngineMenu_GetCellName(index->row);
            break;
        }
        case STATE_ADVENTURE:
        {
            return Adventure_MenuCellName(index);
            break;
        }
        case STATE_OPTIONS:
        {
            if(menu->mainMenu)
            {
                return OptionsMenu_MainCellName(index);
            }
            else
            {
                return OptionsMenu_SlaveCellName(index);
            }
            break;
        }
        case STATE_BATTLE:
        {
            return BattleScreen_MenuCellName(index);
            break;
        }
        case STATE_TITLE_SCREEN:
        {
            return TitleScreen_MenuCellName(index);
            break;
        }
        default:
        {
            return "";
        }
    }
}

const char *GetMenuSectionName(Menu *menu, uint16_t section_index)
{
    GlobalState state = menu->menuState;
    switch(state)
    {
        case STATE_ENGINE_MENU:
        {
            return EngineMenu_GetSectionName();
            break;
        }
        case STATE_ADVENTURE:
        {
            return Adventure_MenuSectionName(section_index);
            break;
        }
        case STATE_OPTIONS:
        {
            if(menu->mainMenu)
                return OptionsMenu_SectionName(section_index);
            else
                return "";
            break;
        }
        case STATE_BATTLE:
        {
            return BattleScreen_MenuSectionName(section_index);
            break;
        }
        case STATE_TITLE_SCREEN:
        {
            return TitleScreen_MenuSectionName(section_index);
            break;
        }
        default:
        {
            return "";
        }
    }
}

const char *GetMenuDescription(Menu *menu, MenuIndex *index)
{
    GlobalState state = menu->menuState;
    switch(state)
    {
        case STATE_ENGINE_MENU:
        {
            return EngineMenu_GetCellName(index->row);
            break;
        }
        case STATE_ADVENTURE:
        {
            return Adventure_MenuCellDescription(index);
            break;
        }
        case STATE_OPTIONS:
        {
            return OptionsMenu_CellDescription(index);
            break;
        }
        case STATE_BATTLE:
        {
            return BattleScreen_MenuCellDescription(index);
            break;
        }
        case STATE_TITLE_SCREEN:
        {
            return TitleScreen_MenuCellDescription(index);
            break;
        }
        default:
        {
            return "";
        }
    }
}

void CallMenuSelectCallback(Menu *menu, ClickRecognizerRef recognizer, Window *window)
{
	if(!menu->menuLayerInitialized)
		return;
	
	MenuIndex index = menu_layer_get_selected_index(menu->menuLayer);
	
	if(index.row < GetMenuCellCount(menu, index.section))
	{
        GlobalState state = menu->menuState;
        switch(state)
        {
            case STATE_ENGINE_MENU:
            {
                return EngineMenu_SelectAction(index.row);
                break;
            }
            case STATE_ADVENTURE:
            {
                Adventure_MenuSelect(&index);
                break;
            }
            case STATE_OPTIONS:
            {
                if(menu->mainMenu)
                    OptionsMenu_Select(&index);
                break;
            }
            case STATE_BATTLE:
            {
                BattleScreen_MenuSelect(&index);
                break;
            }
            case STATE_TITLE_SCREEN:
            {
                TitleScreen_MenuSelect(&index);
                break;
            }
            default:
            {
                return;
            }
        }
	}
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
	if(menu->mainMenu && !IsMenuHidden(menu))
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
    menu->menuState = STATE_NONE;
	return menu;
}

void MenuUpdateProc(struct Layer *layer, GContext *ctx)
{
	GRect bounds = layer_get_bounds(layer);
	DrawContentFrame(ctx, &bounds, GColorBlue);
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
		layer_add_child(menu->topLevelMenuLayer, (Layer*)menu->menuLayer);
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
	if(menu && menu->menuLayerInitialized)
	{
		menu_layer_reload_data(menu->menuLayer);
        if(GetMenuTotalCellCount(menu) > 0)
        {
            MenuIndex index = {.section = 0, .row = 0};
            menu_layer_set_selected_index(menu->menuLayer, index, MenuRowAlignCenter, false);
            if(menu->mainMenu)
            {
				ShowMenuArrow();
			}
        }
		else if(menu->mainMenu)
        {
            HideMenuArrow();
        }
	}
}

typedef struct RegisterMenuState_data
{
    Menu *menu;
    int state;
} RegisterMenuState_data;

void RegisterMenuState_Push(void *data)
{
    RegisterMenuState_data *menuData = (RegisterMenuState_data*)data;
    RegisterMenuState(menuData->menu, menuData->state);
    free(menuData);
    GlobalState_Pop();
}

void QueueRegisterMenuState(Menu *menu, int state)
{
    RegisterMenuState_data *data = calloc(sizeof(RegisterMenuState_data), 1);
    data->menu = menu;
    data->state = state;
    GlobalState_Queue(STATE_REGISTER_MENU_STATE, 0, data);
}

void RegisterMenuState(Menu *menu, int state)
{
    menu->menuState = state;
    ReloadMenu(menu);
}

void Menu_ResetSelection(Menu *menu)
{
    MenuIndex index = {.section = 0, .row = 0};
    menu_layer_set_selected_index(menu->menuLayer, index, MenuRowAlignCenter, false);
}

void TriggerMenu(Menu *menu)
{
	GlobalState_Push(STATE_MENU, 0, menu);
}

void QueueMenu(Menu *menu)
{
    GlobalState_Queue(STATE_MENU, 0, menu);
}
