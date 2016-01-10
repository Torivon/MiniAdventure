#pragma once
	
#include "MiniAdventure.h"

void SetTutorialSeen(bool enable);
bool GetTutorialSeen(void);

void ShowTitleMenu(void);

void RegisterTitleScreen(void);
void TitleScreenAppear(void *data);
void TitleScreenPop(void *data);
