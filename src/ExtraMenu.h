#pragma once

uint16_t ExtraMenu_GetSectionCount(void);
const char *ExtraMenu_GetSectionName(void);
uint16_t ExtraMenu_GetCellCount(void);
const char *ExtraMenu_GetCellName(uint16_t row);
void ExtraMenu_SelectAction(uint16_t row);

void TriggerTutorialDialog(bool now);
