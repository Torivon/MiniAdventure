#pragma once
	
#include "MiniAdventure.h"

void SetTutorialSeen(bool enable);
bool GetTutorialSeen(void);

void TitleScreen_Register(void);
void TitleScreen_Appear(void *data);
void TitleScreen_Pop(void *data);

uint16_t TitleScreen_MenuSectionCount(void);
const char *TitleScreen_MenuSectionName(uint16_t sectionIndex);
uint16_t TitleScreen_MenuCellCount(uint16_t sectionIndex);
const char *TitleScreen_MenuCellName(MenuIndex *index);
const char *TitleScreen_MenuCellDescription(MenuIndex *index);
void TitleScreen_MenuSelect(MenuIndex *index);
