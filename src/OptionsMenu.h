#pragma once
bool GetVibration(void);
void SetVibration(bool enable);
bool GetWorkerApp(void);
void SetWorkerApp(bool enable);
bool GetWorkerCanLaunch(void);
void SetWorkerCanLaunch(bool enable);

void DrawOptionsMenu(void);

void ShowOptionsMenu(void);

bool OptionsMenuIsVisible(void);
void TriggerOptionScreen(void);
void QueueOptionsScreen(void);
