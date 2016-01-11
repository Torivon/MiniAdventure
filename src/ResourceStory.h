#pragma once

#include "AutoSizeConstants.h"
#include "Skills.h"
#include "CombatantClass.h"

typedef enum
{
    STORYUPDATE_COMPUTERANDOM = 0,
    STORYUPDATE_DONOTHING,
    STORYUPDATE_FULLREFRESH,
    STORYUPDATE_WIN,
    STORYUPDATE_TRIGGER_BATTLE,
} ResourceStoryUpdateReturnType;

typedef struct ResourceBattler
{
    char name[MAX_STORY_NAME_LENGTH];
    char description[MAX_STORY_DESC_LENGTH];
    uint16_t image;
    CombatantClass combatantClass;
    SkillList skillList;
    uint16_t vulnerable; // These are bit fields that use the damage type enums
    uint16_t resistant;
    uint16_t immune;
    uint16_t absorb;
} ResourceBattler;

typedef struct BattlerWrapper
{
    bool loaded;
    Skill *loadedSkills[MAX_SKILLS_IN_LIST];
    ResourceBattler battler;
} BattlerWrapper;

typedef struct PersistedResourceStoryState
{
    uint16_t currentLocationIndex;
    uint16_t timeOnPath;
    uint16_t destinationIndex;
    uint16_t pathLength;
    uint16_t encounterChance;
} PersistedResourceStoryState;

void ResourceStory_InitializeCurrent(void);

void ResourceStory_TriggerDialog(uint16_t dialogIndex);
uint16_t ResourceStory_GetOpeningDialogIndex(void);
uint16_t ResourceStory_GetWinDialogIndex(void);


uint16_t ResourceStory_GetCurrentLocationIndex(void);
int ResourceStory_GetCurrentLocationBackgroundImageId(void);
bool ResourceStory_IncrementTimeOnPath(void);
uint16_t ResourceStory_GetTimeOnPath(void);
uint16_t ResourceStory_GetCurrentLocationLength(void);
bool ResourceStory_CurrentLocationIsPath(void);
ResourceStoryUpdateReturnType ResourceStory_UpdateCurrentLocation(void);
const char *ResourceStory_GetCurrentLocationName(void);
uint16_t ResourceStory_GetCurrentAdjacentLocations(void);
ResourceStoryUpdateReturnType ResourceStory_MoveToLocation(uint16_t index);
const char *ResourceStory_GetAdjacentLocationName(uint16_t index);
uint16_t ResourceStory_GetCurrentLocationLength(void);
bool ResourceStory_CurrentLocationIsPath(void);
uint16_t ResourceStory_GetCurrentLocationBaseLevel(void);
uint16_t ResourceStory_GetCurrentLocationEncounterChance(void);
bool ResourceStory_InStory(void);

uint16_t BattlerWrapper_GetUsableSkillCount(BattlerWrapper *wrapper, uint16_t level);
Skill *BattlerWrapper_GetSkillByIndex(BattlerWrapper *wrapper, uint16_t index);
BattlerWrapper *BattlerWrapper_GetPlayerWrapper(void);
BattlerWrapper *BattlerWrapper_GetMonsterWrapper(void);

void ResourceStory_LoadAll(void);
void ResourceStory_LogCurrent(void);
void ResourceStory_FreeAll(void);
void ResourceStory_SetCurrentStory(uint16_t index);
void ResourceStory_ClearCurrentStory(void);

const char *ResourceStory_GetNameByIndex(uint16_t index);
const char *ResourceStory_GetDescriptionByIndex(uint16_t index);

void ResourceStory_GetStoryList(uint16_t *count, uint16_t **buffer);
uint16_t ResourceStory_GetCurrentStoryId(void);
uint16_t ResourceStory_GetCurrentStoryVersion(void);
uint16_t ResourceStory_GetCurrentStoryHash(void);
uint16_t ResourceStory_GetCurrentStoryXPMonstersPerLevel(void);
uint16_t ResourceStory_GetCurrentStoryXPDifferenceScale(void);

void ResourceStory_GetPersistedData(uint16_t *count, uint8_t **buffer);
void ResourceStory_UpdateStoryWithPersistedState(void);

bool ResourceStory_CurrentLocationHasMonster(void);
int ResourceStory_GetCurrentLocationMonster(void);
void ResourceBattler_LoadPlayer(uint16_t classId);

char *ResourceMonster_GetCurrentName(void);
void ResourceMonster_UnloadCurrent(void);
void ResourceMonster_LoadCurrent(uint16_t index);
bool ResourceMonster_Loaded(void);
void ResourceBattler_UnloadPlayer(void);
void ResourceMonster_UnloadCurrent(void);

int16_t ResourceStory_GetStoryIndexById(uint16_t id);

bool ResourceStory_IsLastResourceStoryIdValid(void);
void ResourceStory_SetLastResourceStoryId(uint16_t id);
uint16_t ResourceStory_GetLastResourceStoryId(void);
