//
//  BattleEvents.h
//  
//
//  Created by Jonathan Panttaja on 2/1/16.
//
//
#pragma once

typedef struct BattleEvent BattleEvent;

void BattleEvent_LoadCurrentBattleEvents(uint16_t count, uint16_t *eventIds);
void BattleEvent_FreeCurrentBattleEvents(void);

uint16_t BattleEvent_GetCurrentAvailableBattleEvents(void);
const char *BattleEvent_GetCurrentBattleEventName(uint16_t index);
const char *BattleEvent_GetCurrentBattleEventDescription(uint16_t index);
void BattleEvent_MenuQueue(uint16_t index);

bool BattleEvent_TriggerAutomaticBattleEvents(void);

