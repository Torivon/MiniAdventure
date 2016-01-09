#pragma once

typedef struct Menu Menu;

Menu *CreateMenuLayer(int onScreenX,
					  int yPos,
					  int width,
					  int height,
					  int innerOffset,
					  bool offScreenRight,
					  bool mainMenu);
void InitializeMenuLayer(Menu *menu, Window *window);

bool IsMenuVisible(Menu *menu);
void ShowMenu(void *menu);
void HideMenu(Menu *menu);
void TriggerMenu(Menu *menu);

void CallMenuSelectCallback(Menu *menu, ClickRecognizerRef recognizer, Window *window);

bool IsMenuUsable(Menu *menu);
bool IsMenuHidden(Menu *menu);
bool IsMenuVisible(Menu *menu);

MenuLayer *GetMenuLayer(Menu *menu);

void CleanupMenu(Menu *menu);

typedef uint16_t (*MenuSectionCountCallback)(void);
typedef uint16_t (*MenuCountCallback)(uint16_t section_index);
typedef const char *(*MenuSectionNameCallback)(uint16_t section_index);
typedef const char *(*MenuNameCallback)(MenuIndex *cell_index);
typedef const char *(*MenuDescriptionCallback)(MenuIndex *cell_index);
typedef void(*MenuSelectCallback)(MenuIndex *cell_index);

typedef void (*MenuCellSelectCallback)(void);

typedef struct MenuParameters
{
    MenuSectionNameCallback menuSectionNameCallback;
    MenuSectionCountCallback menuSectionCountCallback;
    MenuCountCallback countCallback;
    MenuNameCallback nameCallback;
    MenuDescriptionCallback descriptionCallback;
    MenuSelectCallback selectCallback;
} MenuParameters;

void RegisterMenuCellCallbacks(Menu *menu, MenuParameters *parameters);
void ClearMenuCellList(Menu *menu);
uint16_t GetMenuCellCount(Menu *menu, uint16_t section_index);
uint16_t GetMenuTotalCellCount(Menu *menu);
void ReloadMenu(Menu *menu);
void Menu_ResetSelection(Menu *menu);

#define MENU_ANIMATION_DURATION 500
