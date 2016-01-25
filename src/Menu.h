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
void QueueMenu(Menu *menu);

void CallMenuSelectCallback(Menu *menu, ClickRecognizerRef recognizer, Window *window);

bool IsMenuUsable(Menu *menu);
bool IsMenuHidden(Menu *menu);
bool IsMenuVisible(Menu *menu);

MenuLayer *GetMenuLayer(Menu *menu);

void CleanupMenu(Menu *menu);

void RegisterMenuState_Push(void *data);
void QueueRegisterMenuState(Menu *menu, int state);
void RegisterMenuState(Menu *menu, int state);
void ClearMenuCellList(Menu *menu);
uint16_t GetMenuCellCount(Menu *menu, uint16_t section_index);
uint16_t GetMenuTotalCellCount(Menu *menu);
void ReloadMenu(Menu *menu);
void Menu_ResetSelection(Menu *menu);

#define MENU_ANIMATION_DURATION 500
