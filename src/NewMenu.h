#pragma once

void InitializeNewMenuLayer(Window *window);

bool IsMenuVisible(void);
void ShowMenu(void);
void HideMenu(void);

bool IsMenuUsable(void);
bool IsMenuHidden(void);
bool IsMenuVisible(void);

MenuLayer *GetNewMenuLayer(void);

void CleanupMenu(void);
