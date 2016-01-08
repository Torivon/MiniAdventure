#pragma once

typedef struct Menu Menu;

void UpdateBatteryLevel(BatteryChargeState chargeState);

Window * InitializeBaseWindow(void);
Menu *GetMainMenu(void);
Menu *GetSlaveMenu(void);
void SetUseSlaveMenu(bool enable);
bool GetUseSlaveMenu(void);
void SetHideMenuOnSelect(bool enable);
bool GetHideMenuOnSelect(void);
void HideBatteryLevel(void);
void ShowBatteryLevel(void);
