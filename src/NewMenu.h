#pragma once

typedef struct Menu Menu;

Menu *CreateMenuLayer(int onScreenX,
					  int yPos,
					  int width,
					  int height,
					  int innerOffset,
					  bool offScreenRight,
					  bool mainMenu);
void InitializeNewMenuLayer(Menu *menu, Window *window);

bool IsMenuVisible(Menu *menu);
void ShowMenu(void *menu);
void HideMenu(Menu *menu);
void TriggerMenu(Menu *menu);

void CallNewMenuSelectCallback(Menu *menu, ClickRecognizerRef recognizer, Window *window);

bool IsMenuUsable(Menu *menu);
bool IsMenuHidden(Menu *menu);
bool IsMenuVisible(Menu *menu);

MenuLayer *GetNewMenuLayer(Menu *menu);

void CleanupMenu(Menu *menu);

typedef uint16_t (*MenuCountCallback)(void);
typedef const char *(*MenuNameCallback)(int row);
typedef const char *(*MenuDescriptionCallback)(int row);
typedef void(*MenuSelectCallback)(int row);

typedef void (*MenuCellSelectCallback)(void);

typedef struct MenuCellDescription
{
	char *name;
	char *description;
	MenuCellSelectCallback callback;
} MenuCellDescription;

void RegisterMenuCellCallbacks(Menu *menu, MenuCountCallback countCallback, MenuNameCallback nameCallback, MenuDescriptionCallback descriptionCallback, MenuSelectCallback selectCallback);
void RegisterMenuCellList(Menu *menu, MenuCellDescription *list, uint16_t count);
void ClearMenuCellList(Menu *menu);
uint16_t GetMenuCellCount(Menu *menu);
void ReloadMenu(Menu *menu);

#define MENU_ANIMATION_DURATION 500
