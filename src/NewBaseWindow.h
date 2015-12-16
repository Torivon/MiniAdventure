#pragma once

typedef struct Menu Menu;

void UpdateNewClock(void);

Window * InitializeNewBaseWindow(void);
bool UsingNewWindow(void);
Menu *GetMainMenu(void);
