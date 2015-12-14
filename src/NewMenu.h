#pragma once

void InitializeNewMenuLayer(Window *window);

bool IsMenuVisible(void);
void ShowMenu(void);
void HideMenu(void);
void TriggerMenu(void);

void CallNewMenuSelectCallback(ClickRecognizerRef recognizer, Window *window);

bool IsMenuUsable(void);
bool IsMenuHidden(void);
bool IsMenuVisible(void);

MenuLayer *GetNewMenuLayer(void);

void CleanupMenu(void);

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

void RegisterMenuCellCallbacks(MenuCountCallback countCallback, MenuNameCallback nameCallback, MenuDescriptionCallback descriptionCallback, MenuSelectCallback selectCallback);
void RegisterMenuCellList(MenuCellDescription *list, uint16_t count);
void ClearMenuCellList(void);
uint16_t GetMenuCellCount(void);

#define MENU_ANIMATION_DURATION 500
