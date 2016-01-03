#pragma once

bool AdventureWindowIsVisible(void);

void SetUpdateDelay(void);

void RefreshAdventure(void);
void ResetGame(void);

extern bool gUpdateAdventure;
void TriggerAdventureScreen(void);
void QueueAdventureScreen(void);
