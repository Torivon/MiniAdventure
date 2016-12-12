#pragma once

uint16_t EngineMenu_GetSectionCount(void);
const char *EngineMenu_GetSectionName(void);
uint16_t EngineMenu_GetCellCount(void);
const char *EngineMenu_GetCellName(uint16_t row);
void EngineMenu_SelectAction(uint16_t row);

void TriggerTutorialDialog(bool now);
void EngineMenu_SubMenu_Trigger(void);

