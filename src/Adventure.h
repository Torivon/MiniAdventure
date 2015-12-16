#pragma once

void ToggleVibration(void);
const char *UpdateVibrationText(void);

bool AdventureWindowIsVisible(void);

void SetUpdateDelay(void);

void RefreshAdventure(void);
void ResetGame(void);

extern bool gUpdateAdventure;
void TriggerAdventureScreen(void);
