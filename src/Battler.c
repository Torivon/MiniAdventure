//
//  Battler.c
//  
//
//  Created by Jonathan Panttaja on 1/26/16.
//
//

#include <pebble.h>
#include "AutoSizeConstants.h"
#include "BattleEvents.h"
#include "Battler.h"
#include "BinaryResourceLoading.h"
#include "CombatantClass.h"
#include "Events.h"
#include "ImageMap.h"
#include "Skills.h"
#include "Story.h"

typedef struct Battler
{
    char name[MAX_STORY_NAME_LENGTH];
    char description[MAX_STORY_DESC_LENGTH];
    uint16_t image;
    CombatantClass combatantClass;
    SkillList skillList;
    uint16_t battleEventCount;
    uint16_t battleEvents[MAX_BATTLE_EVENTS];
    uint16_t event;
    uint16_t vulnerable; // These are bit fields that use the damage type enums
    uint16_t resistant;
    uint16_t immune;
    uint16_t absorb;
    uint16_t statusImmunities;
    uint16_t aiType;
} Battler;

typedef struct BattlerWrapper
{
    bool loaded;
    Skill *loadedSkills[MAX_SKILLS_IN_LIST];
    Event *event;
    Battler battler;
} BattlerWrapper;

static BattlerWrapper currentMonster = {0};
static BattlerWrapper playerClass = {0};

BattlerWrapper *BattlerWrapper_GetPlayerWrapper(void)
{
    return &playerClass;
}

BattlerWrapper *BattlerWrapper_GetMonsterWrapper(void)
{
    return &currentMonster;
}

Skill *BattlerWrapper_GetSkillByIndex(BattlerWrapper *wrapper, uint16_t index)
{
    return wrapper->loadedSkills[index];
}

uint16_t BattlerWrapper_GetUsableSkillCount(BattlerWrapper *wrapper, uint16_t level)
{
    int count;
    for(count = 0; count < wrapper->battler.skillList.count; ++count)
    {
        if(wrapper->battler.skillList.entries[count].level > level)
            break;
    }
    return count;
}

int BattlerWrapper_GetImage(BattlerWrapper *wrapper)
{
    if(wrapper->loaded)
    {
        return wrapper->battler.image;
    }
    else
    {
        return -1;
    }
}

CombatantClass *BattlerWrapper_GetCombatantClass(BattlerWrapper *wrapper)
{
    return &wrapper->battler.combatantClass;
}

SkillList *BattlerWrapper_GetSkillList(BattlerWrapper *wrapper)
{
    return &wrapper->battler.skillList;
}

void Battler_UnloadBattler(BattlerWrapper *wrapper)
{
    if(!wrapper->loaded)
        return;
    
    wrapper->loaded = false;
    for(int i = 0; i < wrapper->battler.skillList.count; ++i)
    {
        Skill_Free(wrapper->loadedSkills[i]);
        wrapper->loadedSkills[i] = NULL;
    }
    Event_Free(wrapper->event);
    wrapper->event = NULL;
}

void Monster_UnloadCurrent(void)
{
    BattleEvent_FreeCurrentBattleEvents();
    Battler_UnloadBattler(&currentMonster);
}

void Battler_UnloadPlayer(void)
{
    Battler_UnloadBattler(&playerClass);
}

bool Battler_LoadBattler(BattlerWrapper *wrapper, uint16_t logical_index)
{
    ResHandle currentStoryData = Story_GetCurrentResHandle();
    ResourceLoadStruct(currentStoryData, logical_index, (uint8_t*)(&(wrapper->battler)), sizeof(Battler), "Battler");
    
    wrapper->battler.image = ImageMap_GetIdByIndex(wrapper->battler.image);
    for(int i = 0; i < wrapper->battler.skillList.count; ++i)
    {
        wrapper->loadedSkills[i] = Skill_Load(wrapper->battler.skillList.entries[i].id);
        wrapper->battler.skillList.entries[i].id = i;
    }
    
    wrapper->loaded = true;
    
    if(wrapper->battler.event > 0)
    {
        wrapper->event = Event_Load(wrapper->battler.event);
        return Event_CheckPrerequisites(wrapper->event);
    }
    
    return true;
}

bool Monster_LoadCurrent(uint16_t logical_index)
{
    bool returnval = Battler_LoadBattler(&currentMonster, logical_index);
    if(returnval)
    {
        BattleEvent_LoadCurrentBattleEvents(currentMonster.battler.battleEventCount, currentMonster.battler.battleEvents);
    }
    return returnval;
}

void Battler_LoadPlayer(uint16_t classId)
{
    Battler_LoadBattler(&playerClass, Story_GetClassByIndex(classId));
}

const char *BattlerWrapper_GetBattlerName(BattlerWrapper *wrapper)
{
    if(wrapper->loaded)
    {
        return wrapper->battler.name;
    }
    else
    {
        return "None";
    }
}

const char *Monster_GetCurrentName(void)
{
    return BattlerWrapper_GetBattlerName(&currentMonster);
}

int16_t Monster_GetCurrentImage(void)
{
    if(currentMonster.loaded)
    {
        return currentMonster.battler.image;
    }
    else
    {
        return -1;
    }
}

bool Monster_Loaded(void)
{
    return currentMonster.loaded;
}

void BattlerWrapper_TriggerEvent(BattlerWrapper* wrapper)
{
    if(wrapper && wrapper->event)
        Event_TriggerEvent(wrapper->event, false);
}

bool BattlerWrapper_CheckImmunity(BattlerWrapper *wrapper, uint16_t damageType)
{
    return wrapper->battler.immune & damageType;
}

bool BattlerWrapper_CheckVulnerability(BattlerWrapper *wrapper, uint16_t damageType)
{
    return wrapper->battler.vulnerable & damageType;
}

bool BattlerWrapper_CheckResistance(BattlerWrapper *wrapper, uint16_t damageType)
{
    return wrapper->battler.resistant & damageType;
}

bool BattlerWrapper_CheckAbsorption(BattlerWrapper *wrapper, uint16_t damageType)
{
    return wrapper->battler.absorb & damageType;
}

bool BattlerWrapper_CheckStatusImmunity(BattlerWrapper *wrapper, uint16_t status)
{
    return wrapper->battler.statusImmunities & status;
}

uint16_t BattlerWrapper_GetAIType(BattlerWrapper *wrapper)
{
    return wrapper->battler.aiType;
}
