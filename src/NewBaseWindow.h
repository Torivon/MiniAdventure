#pragma once

typedef struct Menu Menu;

void UpdateNewClock(void);
void UpdateBatteryLevel(int current);

Window * InitializeNewBaseWindow(void);
bool UsingNewWindow(void);
Menu *GetMainMenu(void);
Menu *GetSlaveMenu(void);
void SetUseSlaveMenu(bool enable);
bool GetUseSlaveMenu(void);
void SetHideMenuOnSelect(bool enable);
bool GetHideMenuOnSelect(void);
