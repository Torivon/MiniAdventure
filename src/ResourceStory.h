#pragma once

#include "AutoSizeConstants.h"

typedef struct Skill Skill;
typedef struct SkillList SkillList;

typedef enum
{
    STORYUPDATE_COMPUTERANDOM = 0,
    STORYUPDATE_DONOTHING,
    STORYUPDATE_FULLREFRESH,
} ResourceStoryUpdateReturnType;

void ResourceStory_InitializeCurrent(void);

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
void ResourceStory_GetPersistedData(uint16_t *count, uint8_t **buffer);
void ResourceStory_UpdateStoryWithPersistedState(void);

bool ResourceStory_CurrentLocationHasMonster(void);
int ResourceStory_GetCurrentLocationMonster(void);
SkillList *ResourceStory_GetCurrentMonsterSkillList(void);
CombatantClass *ResourceStory_GetCurrentMonsterCombatantClass(void);
CombatantClass *ResourceStory_GetCurrentPlayerCombatantClass(void);
SkillList *ResourceStory_GetCurrentPlayerSkillList(void);
void ResourceBattler_LoadPlayer(uint16_t classId);
Skill *ResourceBattler_GetPlayerSkillByID(int index);

char *ResourceMonster_GetCurrentName(void);
void ResourceMonster_UnloadCurrent(void);
void ResourceMonster_LoadCurrent(uint16_t index);
bool ResourceMonster_Loaded(void);
int ResourceStory_GetCurrentMonsterImage(void);
Skill *ResourceStory_GetSkillByID(int index);
Skill *ResourceStory_GetLoadedSkillByID(bool player, int index);
void ResourceBattler_UnloadPlayer(void);
void ResourceMonster_UnloadCurrent(void);

