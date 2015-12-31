#pragma once

#include "AutoSizeConstants.h"

typedef struct MonsterDef MonsterDef;

typedef void (*StoryInitializeFunction)(void);

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
void ResourceStory_MoveToLocation(uint16_t index);
const char *ResourceStory_GetAdjacentLocationName(uint16_t index);
uint16_t ResourceStory_GetCurrentLocationLength(void);
bool ResourceStory_CurrentLocationIsPath(void);

void ResourceStory_LoadAll(void);
void ResourceStory_LogCurrent(void);
void ResourceStory_FreeAll(void);
void ResourceStory_SetCurrentStory(uint16_t index);
void ResourceStory_ClearCurrentStory(void);

const char *ResourceStory_GetNameByIndex(uint16_t index);
const char *ResourceStory_GetDescriptionByIndex(uint16_t index);
