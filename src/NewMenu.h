#pragma once

void InitializeNewMenuLayer(Window *window);

bool IsMenuVisible(void);
void ShowMenu(void);
void HideMenu(void);

void CallNewMenuSelectCallback(ClickRecognizerRef recognizer, Window *window);

bool IsMenuUsable(void);
bool IsMenuHidden(void);
bool IsMenuVisible(void);

MenuLayer *GetNewMenuLayer(void);

void CleanupMenu(void);

typedef void (*MenuCellSelectCallback)(void);

typedef struct MenuCellDescription
{
	char *name;
	char *description;
	MenuCellSelectCallback callback;
} MenuCellDescription;

void RegisterMenuCellList(MenuCellDescription *list, uint16_t count);
void ClearMenuCellList(void);
uint16_t GetMenuCellCount(void);

#define MENU_ANIMATION_DURATION 500
