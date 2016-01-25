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

void TriggerAdventureScreen(void);
void QueueAdventureScreen(void);

uint16_t Adventure_MenuSectionCount(void);
const char *Adventure_MenuSectionName(uint16_t sectionIndex);
uint16_t Adventure_MenuCellCount(uint16_t sectionIndex);
const char *Adventure_MenuCellName(MenuIndex *index);
const char *Adventure_MenuCellDescription(MenuIndex *index);
void Adventure_MenuSelect(MenuIndex *index);
