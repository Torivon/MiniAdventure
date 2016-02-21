//
//  BattleEvents.c
//  
//
//  Created by Jonathan Panttaja on 2/1/16.
//
//
#include <pebble.h>

#include "AutoSizeConstants.h"
#include "BattleEvents.h"

typedef struct BattleEvent
{
    char name[MAX_STORY_NAME_LENGTH];
    char menuDescription[MAX_STORY_DESC_LENGTH];
    uint16_t automatic; // boolean
    uint16_t dialog;
    uint16_t prerequisiteCount;
    uint16_t prerequisiteType[MAX_BATTLE_EVENT_PREREQS];
    uint16_t prerequisiteValue[MAX_BATTLE_EVENT_PREREQS];
    uint16_t battlerSwitch; // boolean
    uint16_t newBattler;
    uint16_t fullHealOnBattlerChange; // boolean
} BattleEvent;

