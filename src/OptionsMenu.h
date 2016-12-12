#pragma once
bool GetVibration(void);
void SetVibration(bool enable);
bool GetWorkerApp(void);
void SetWorkerApp(bool enable);
bool GetWorkerCanLaunch(void);
void SetWorkerCanLaunch(bool enable);
bool GetAllowActivity(void);
void SetAllowActivity(bool enable);

void ShowOptionsMenu(void);

bool OptionsMenuIsVisible(void);
void TriggerOptionScreen(void);
void QueueOptionsScreen(void);
void OptionScreenPush(void *data);
void OptionScreenAppear(void *data);
void OptionScreenPop(void *data);

uint16_t OptionsMenu_CellCount(uint16_t sectionIndex);
const char *OptionsMenu_SlaveCellName(MenuIndex *index);
const char *OptionsMenu_CellDescription(MenuIndex *index);
const char *OptionsMenu_MainCellName(MenuIndex *index);
const char *OptionsMenu_SectionName(uint16_t sectionIndex);
uint16_t OptionsMenu_SectionCount(void);
void OptionsMenu_Select(MenuIndex *index);
