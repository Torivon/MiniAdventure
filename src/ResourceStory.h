#pragma once

#include "AutoSizeConstants.h"

typedef struct MonsterDef MonsterDef;

typedef void (*StoryInitializeFunction)(void);

typedef struct ResourceLocation
{
    char name[MAX_STORY_NAME_LENGTH];
    uint16_t adjacentLocationCount;
    uint16_t adjacentLocations[MAX_ADJACENT_LOCATIONS];
    uint16_t backgroundImageCount;
    uint16_t backgroundImages[MAX_BACKGROUND_IMAGES];
} ResourceLocation;

typedef struct ResourceStory
{
	uint16_t id;
	uint16_t version;
    char name[MAX_STORY_NAME_LENGTH];
    char description[MAX_STORY_DESC_LENGTH];
    uint16_t start_location;
} ResourceStory;

typedef struct PersistedResourceStoryState
{
    int currentLocationIndex;
} PersistedResourceStoryState;

typedef struct ResourceStoryState
{
    bool needsSaving;
    PersistedResourceStoryState persistedResourceStoryState;
} ResourceStoryState;

void ResourceStory_InitializeCurrent(void);

uint16_t ResourceStory_GetCurrentLocationIndex(void);
int ResourceStory_GetCurrentLocationBackgroundImageId(void);
const char *ResourceStory_GetCurrentLocationName(void);
uint16_t ResourceStory_GetCurrentAdjacentLocations(void);
void ResourceStory_MoveToLocation(uint16_t index);

void ResourceStory_Load(int resourceId);
void ResourceStory_LogCurrent(void);
void ResourceStory_FreeCurrent(void);

ResourceLocation *ResourceLocation_Load(uint16_t index);
void ResourceLocation_Log(ResourceLocation *location);
void ResourceLocation_Free(ResourceLocation *location);
