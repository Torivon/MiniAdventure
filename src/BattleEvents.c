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
#include "Events.h"
#include "Logging.h"
#include "Skills.h"
#include "Story.h"

typedef struct BattleEvent
{
    char name[MAX_STORY_NAME_LENGTH];
    char menuDescription[MAX_STORY_DESC_LENGTH];
    uint16_t automatic; // boolean
    uint16_t dialog;
    uint16_t subEvent;
    uint16_t skill;
    uint16_t prerequisiteCount;
    uint16_t prerequisiteType[MAX_BATTLE_EVENT_PREREQS];
    uint16_t prerequisiteValue[MAX_BATTLE_EVENT_PREREQS];
    uint16_t battlerSwitch; // boolean
    uint16_t newBattler;
    uint16_t fullHealOnBattlerChange; // boolean
} BattleEvent;

static uint16_t currentBattleEventCount = 0;
static BattleEvent *currentBattleEvents[MAX_BATTLE_EVENTS] = {0};
static Event *currentBattleEventSubEvents[MAX_BATTLE_EVENTS] = {0};
static Skill *currentBattleEventSkills[MAX_BATTLE_EVENTS] = {0};

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
        if(currentBattleEvents[i]->subEvent)
            currentBattleEventSubEvents[i] = Event_Load(currentBattleEvents[i]->subEvent);
        if(currentBattleEvents[i]->skill)
            currentBattleEventSkills[i] = Skill_Load(currentBattleEvents[i]->skill);
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
        if(currentBattleEventSubEvents[i])
        {
            Event_Free(currentBattleEventSubEvents[i]);
            currentBattleEventSubEvents[i] = NULL;
        }
        if(currentBattleEventSkills[i])
        {
            Skill_Free(currentBattleEventSkills[i]);
            currentBattleEventSkills[i] = NULL;
        }
    }
}

bool BattleEvent_CheckPrerequisites(BattleEvent *battleEvent, Event *subEvent)
{
    bool returnval = true;
    
    if(subEvent)
    {
        returnval = returnval && Event_CheckPrerequisites(subEvent);
    }
    
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
            case BATTLE_EVENT_TYPE_PLAYER_HEALTH_BELOW_PERCENT:
            {
                BattleActor *playerActor = GetPlayerActor();
                uint16_t percent = playerActor->currentHealth * 100 / playerActor->maxHealth;
                returnval = returnval && (percent <= battleEvent->prerequisiteValue[i]);
                break;
            }
            case BATTLE_EVENT_TYPE_TIME_ABOVE:
            {
                BattleActor *monsterActor = GetMonsterActor();
                returnval = returnval && monsterActor->timeInCombat > battleEvent->prerequisiteValue[i];
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

// Trigger is only used for automatic events. Any skills triggered by these come from the enemy
void BattleEvent_Trigger(BattleEvent *battleEvent, Event *subEvent, Skill *skill)
{
    if(battleEvent->dialog > 0)
    {
        Dialog_TriggerFromResource(Story_GetCurrentResHandle(), battleEvent->dialog);
    }
    if(subEvent)
    {
        Event_TriggerEvent(subEvent, false);
    }
    if(skill)
    {
        ExecuteSkill(skill, GetMonsterActorWrapper(), GetPlayerActorWrapper());
    }
    if(battleEvent->battlerSwitch)
    {
        Battle_InitializeNewMonster(battleEvent->newBattler, battleEvent->fullHealOnBattlerChange);
    }
    return;
}

// Queue is only used for active events. Any skills trigger by these come from the player
void BattleEvent_Queue(BattleEvent *battleEvent, Event *subEvent, Skill *skill)
{
    if(battleEvent->dialog > 0)
    {
        Dialog_QueueFromResource(Story_GetCurrentResHandle(), battleEvent->dialog);
    }
    if(subEvent)
    {
        Event_TriggerEvent(subEvent, false);
    }
    if(skill)
    {
        ExecuteSkill(skill, GetPlayerActorWrapper(), GetMonsterActorWrapper());
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
        if(!currentBattleEvents[i]->automatic && BattleEvent_CheckPrerequisites(currentBattleEvents[i], currentBattleEventSubEvents[i]))
        {
            if(newIndex == index)
            {
                indexToUse = i;
                break;
            }
            ++newIndex;
        }
    }
    
    BattleEvent_Queue(currentBattleEvents[indexToUse], currentBattleEventSubEvents[indexToUse], currentBattleEventSkills[indexToUse]);
}

bool BattleEvent_TriggerAutomaticBattleEvents(void)
{
    for(int i = 0; i < currentBattleEventCount; ++i)
    {
        if(currentBattleEvents[i]->automatic && BattleEvent_CheckPrerequisites(currentBattleEvents[i], currentBattleEventSubEvents[i]))
        {
            BattleEvent_Trigger(currentBattleEvents[i], currentBattleEventSubEvents[i], currentBattleEventSkills[i]);
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
            if(!currentBattleEvents[i]->automatic && BattleEvent_CheckPrerequisites(currentBattleEvents[i], currentBattleEventSubEvents[i]))
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
        if(!currentBattleEvents[i]->automatic && BattleEvent_CheckPrerequisites(currentBattleEvents[i], currentBattleEventSubEvents[i]))
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
        if(!currentBattleEvents[i]->automatic && BattleEvent_CheckPrerequisites(currentBattleEvents[i], currentBattleEventSubEvents[i]))
        {
            if(currentIndex == index)
                return currentBattleEvents[i]->menuDescription;
            ++currentIndex;
        }
    }
    
    return "None";
}

