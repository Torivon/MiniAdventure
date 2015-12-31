#pragma once

typedef struct MonsterDef MonsterDef;

typedef void (*StoryInitializeFunction)(void);

#define MAX_STORY_NAME_LENGTH 256
#define MAX_STORY_DESC_LENGTH 256

typedef struct ResourceStory
{
	uint16_t id;
	uint16_t version;
    char name[MAX_STORY_NAME_LENGTH];
    char description[MAX_STORY_DESC_LENGTH];
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


void ResourceStory_Load(int resourceId);
void ResourceStory_LogCurrent(void);
void ResourceStory_FreeCurrent(void);
