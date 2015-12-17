#pragma once
#include "MiniAdventure.h"

#if COMPILE_OLD_UILAYERS
void ShowMainMenu(void);
void ShowTestMenu(void);
void ShowTestMenu2(void);

#if ALLOW_TEST_MENU
void UpdateMinFreeMemory();
#else
#define UpdateMinFreeMemory()
#endif
#endif