#pragma once

void SetUpdateDelay(void);

void ResetGame(void);
void ResetGamePush(void *data);
void ResetGamePop(void *data);

void AdventureScreenPush(void *data);
void AdventureScreenAppear(void *data);
void UpdateAdventure(void *data);
void AdventureScreenDisappear(void *data);
void AdventureScreenPop(void *data);


void QueueAdventureScreen(void);
