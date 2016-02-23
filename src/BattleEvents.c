//
//  BattleEvents.c
//  
//
//  Created by Jonathan Panttaja on 2/1/16.
//
//
#include <pebble.h>

#include "AutoBattleEventConstants.h"
#include "AutoSizeConstants.h"
#include "Battle.h"
#include "BattleEvents.h"
#include "BinaryResourceLoading.h"
#include "DialogFrame.h"
#include "Logging.h"
#include "Skills.h"
#include "Story.h"

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

static uint16_t currentBattleEventCount = 0;
static BattleEvent *currentBattleEvents[MAX_BATTLE_EVENTS] = {0};

BattleEvent *BattleEvent_Load(uint16_t logical_index)
{
    if(logical_index == 0)
        return NULL;
    
    BattleEvent *battleEvent = calloc(sizeof(BattleEvent), 1);
    ResourceLoadStruct(Story_GetCurrentResHandle(), logical_index, (uint8_t*)battleEvent, sizeof(BattleEvent), "BattleEvent");
    return battleEvent;
}

void BattleEvent_Free(BattleEvent *battleEvent)
{
    if(battleEvent)
        free(battleEvent);
}

void BattleEvent_LoadCurrentBattleEvents(uint16_t count, uint16_t *eventIds)
{
    currentBattleEventCount = count;
    for(int i = 0; i < currentBattleEventCount; ++i)
    {
        currentBattleEvents[i] = BattleEvent_Load(eventIds[i]);
    }
}

void BattleEvent_FreeCurrentBattleEvents(void)
{
    for(int i = 0; i < MAX_BATTLE_EVENTS; ++i)
    {
        if(currentBattleEvents[i])
        {
            BattleEvent_Free(currentBattleEvents[i]);
            currentBattleEvents[i] = NULL;
        }
    }
}

bool BattleEvent_CheckPrerequisites(BattleEvent *battleEvent)
{
    bool returnval = true;
    
    for(int i = 0; i < battleEvent->prerequisiteCount; ++i)
    {
        switch (battleEvent->prerequisiteType[i])
        {
            case BATTLE_EVENT_TYPE_MONSTER_HEALTH_BELOW_PERCENT:
            {
                BattleActor *monsterActor = GetMonsterActor();
                uint16_t percent = monsterActor->currentHealth * 100 / monsterActor->maxHealth;
                returnval = returnval && (percent <= battleEvent->prerequisiteValue[i]);
                break;
            }
            default:
            {
                break;
            }
        }
    }
    
    return returnval;
}

void BattleEvent_Trigger(BattleEvent *battleEvent)
{
    if(battleEvent->dialog > 0)
    {
        Dialog_TriggerFromResource(Story_GetCurrentResHandle(), battleEvent->dialog);
    }
    if(battleEvent->battlerSwitch)
    {
        Battle_InitializeNewMonster(battleEvent->newBattler, battleEvent->fullHealOnBattlerChange);
    }
    return;
}

void BattleEvent_Queue(BattleEvent *battleEvent)
{
    if(battleEvent->dialog > 0)
    {
        Dialog_QueueFromResource(Story_GetCurrentResHandle(), battleEvent->dialog);
    }
    if(battleEvent->battlerSwitch)
    {
        Battle_InitializeNewMonster(battleEvent->newBattler, battleEvent->fullHealOnBattlerChange);
    }
    return;
}

void BattleEvent_MenuQueue(uint16_t index)
{
    uint16_t newIndex = 0;
    uint16_t indexToUse = 0;
    for(int i = 0; i < currentBattleEventCount; ++i)
    {
        if(!currentBattleEvents[i]->automatic && BattleEvent_CheckPrerequisites(currentBattleEvents[i]))
        {
            if(newIndex == index)
            {
                indexToUse = i;
                break;
            }
            ++newIndex;
        }
    }
    
    BattleEvent_Queue(currentBattleEvents[indexToUse]);
}

bool BattleEvent_TriggerAutomaticBattleEvents(void)
{
    for(int i = 0; i < currentBattleEventCount; ++i)
    {
        if(currentBattleEvents[i]->automatic && BattleEvent_CheckPrerequisites(currentBattleEvents[i]))
        {
            BattleEvent_Trigger(currentBattleEvents[i]);
            DEBUG_LOG("Triggering battle event");
            return true;
        }
    }
    return false;
}

uint16_t BattleEvent_GetCurrentAvailableBattleEvents(void)
{
    if(currentBattleEventCount > 0)
    {
        uint16_t count = 0;
        for(int i = 0; i < currentBattleEventCount; ++i)
        {
            if(!currentBattleEvents[i]->automatic && BattleEvent_CheckPrerequisites(currentBattleEvents[i]))
                ++count;
        }
        return count;
    }
    else
    {
        return 0;
    }
}

const char *BattleEvent_GetCurrentBattleEventName(uint16_t index)
{
    uint16_t currentIndex = 0;
    for(int i = 0; i < currentBattleEventCount; ++i)
    {
        if(!currentBattleEvents[i]->automatic && BattleEvent_CheckPrerequisites(currentBattleEvents[i]))
        {
            if(currentIndex == index)
                return currentBattleEvents[i]->name;
            ++currentIndex;
        }
    }
    
    return "None";
}

const char *BattleEvent_GetCurrentBattleEventDescription(uint16_t index)
{
    uint16_t currentIndex = 0;
    for(int i = 0; i < currentBattleEventCount; ++i)
    {
        if(!currentBattleEvents[i]->automatic && BattleEvent_CheckPrerequisites(currentBattleEvents[i]))
        {
            if(currentIndex == index)
                return currentBattleEvents[i]->menuDescription;
            ++currentIndex;
        }
    }
    
    return "None";
}

