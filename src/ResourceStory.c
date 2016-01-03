#include "pebble.h"

#include "AutoImageMap.h"
#include "AutoSizeConstants.h"
#include "BinaryResourceLoading.h"
#include "CombatantClass.h"
#include "Utils.h"
#include "Logging.h"
#include "NewBattle.h"
#include "ResourceStory.h"
#include "Skills.h"
#include "StoryList.h"


/********************* PREDECLARATIONS *********************************/
static ResHandle ResourceStory_GetCurrentResHandle(void);

/********************* RESOURCE LOCATION *******************************/
typedef struct ResourceLocation
{
    char name[MAX_STORY_NAME_LENGTH];
    uint16_t adjacentLocationCount;
    uint16_t adjacentLocations[MAX_ADJACENT_LOCATIONS];
    uint16_t backgroundImageCount;
    uint16_t backgroundImages[MAX_BACKGROUND_IMAGES];
    uint16_t length;
    uint16_t baseLevel;
    uint16_t encounterChance;
    uint16_t monsterCount;
    uint16_t monsters[MAX_MONSTERS];
} ResourceLocation;

static ResourceLocation *currentLocation = NULL;
static ResourceLocation *adjacentLocations[MAX_ADJACENT_LOCATIONS] = {0};

static ResourceLocation *ResourceLocation_Load(uint16_t logical_index)
{
    ResHandle currentStoryData = ResourceStory_GetCurrentResHandle();
    ResourceLocation *newLocation = calloc(sizeof(ResourceLocation), 1);
    ResourceLoadStruct(currentStoryData, logical_index, (uint8_t*)newLocation, sizeof(ResourceLocation), "ResourceLocation");
    
    for(int i = 0; i < newLocation->backgroundImageCount; ++i)
    {
        newLocation->backgroundImages[i] = autoImageMap[newLocation->backgroundImages[i]];
    }
    
    return newLocation;
}

uint16_t ResourceStory_GetCurrentLocationEncounterChance(void)
{
    if(currentLocation)
    {
        return currentLocation->encounterChance;
    }
    else
    {
        return 0;
    }
}

uint16_t ResourceStory_GetCurrentLocationBaseLevel(void)
{
    if(currentLocation)
    {
        return currentLocation->baseLevel;
    }
    else
    {
        return 1;
    }
}

static void ResourceLocation_Free(ResourceLocation *location)
{
    if(location)
    {
        free(location);
    }
}

#if DEBUGLOGGING > 1
static void ResourceLocation_Log(ResourceLocation *location)
{
    DEBUG_VERBOSE_LOG("ResourceLocation: %s", location->name);
    for(int i = 0; i < location->adjacentLocationCount; ++i)
    {
        DEBUG_VERBOSE_LOG("Adjacent Location: %d", location->adjacentLocations[i]);
    }
    for(int i = 0; i < location->backgroundImageCount; ++i)
    {
        DEBUG_VERBOSE_LOG("Background: %d", location->backgroundImages[i]);
    }
}
#endif

static void ResourceLocation_LoadAdjacentLocations(void)
{
    for(int i = 0; i < currentLocation->adjacentLocationCount; ++i)
    {
        adjacentLocations[i] = ResourceLocation_Load(currentLocation->adjacentLocations[i]);
    }
}

void ResourceLocation_FreeAdjacentLocations(void)
{
    for(int i = 0; i < MAX_ADJACENT_LOCATIONS; ++i)
    {
        if(adjacentLocations[i])
        {
            ResourceLocation_Free(adjacentLocations[i]);
            adjacentLocations[i] = NULL;
        }
    }
}

uint16_t ResourceStory_GetCurrentAdjacentLocations(void)
{
    if(currentLocation)
    return currentLocation->adjacentLocationCount;
    else
    return 0;
}

const char *ResourceStory_GetAdjacentLocationName(uint16_t index)
{
    if(adjacentLocations[index])
    {
        return adjacentLocations[index]->name;
    }
    
    return "None";
}

int ResourceStory_GetCurrentLocationBackgroundImageId(void)
{
    if(currentLocation && currentLocation->backgroundImageCount)
    {
        uint16_t index = Random(currentLocation->backgroundImageCount);
        return currentLocation->backgroundImages[index];
    }
    else
    {
        return -1;
    }
}

bool ResourceStory_CurrentLocationIsPath(void)
{
    if(currentLocation)
    {
        return currentLocation->length > 0;
    }
    
    return false;
}
/********************* RESOURCE SKILL ******************************/
void ResourceSkill_Free(Skill *skill)
{
    free(skill);
}

Skill *ResourceSkill_Load(uint16_t logical_index)
{
    ResHandle currentStoryData = ResourceStory_GetCurrentResHandle();
    Skill *newSkill = calloc(sizeof(Skill), 1);
    ResourceLoadStruct(currentStoryData, logical_index, (uint8_t*)newSkill, sizeof(Skill), "Skill");
    return newSkill;
}

/********************* RESOURCE MONSTER ******************************/

typedef struct ResourceMonster
{
    char name[MAX_STORY_NAME_LENGTH];
    uint16_t image;
    CombatantClass combatantClass;
    SkillList skillList;
} ResourceMonster;

typedef struct MonsterWrapper
{
    bool loaded;
    ResourceMonster monster;
} MonsterWrapper;

MonsterWrapper currentMonster = {0};
Skill *loadedSkills[MAX_SKILLS_IN_LIST];

void ResourceMonster_UnloadCurrent(void)
{
    if(!currentMonster.loaded)
        return;
    
    currentMonster.loaded = false;
    for(int i = 0; i < currentMonster.monster.skillList.count; ++i)
    {
        ResourceSkill_Free(loadedSkills[i]);
        loadedSkills[i] = NULL;
    }
}

void ResourceMonster_LoadCurrent(uint16_t logical_index)
{
    ResHandle currentStoryData = ResourceStory_GetCurrentResHandle();
    ResourceLoadStruct(currentStoryData, logical_index, (uint8_t*)(&(currentMonster.monster)), sizeof(ResourceMonster), "ResourceMonster");

    currentMonster.monster.image = autoImageMap[currentMonster.monster.image];
    for(int i = 0; i < currentMonster.monster.skillList.count; ++i)
    {
        loadedSkills[i] = ResourceSkill_Load(currentMonster.monster.skillList.entries[i].id);
        currentMonster.monster.skillList.entries[i].id = i;
    }
    currentMonster.loaded = true;
}

Skill *ResourceStory_GetSkillByID(int index)
{
    return loadedSkills[index];
}

char *ResourceMonster_GetCurrentName(void)
{
    if(currentMonster.loaded)
    {
        return currentMonster.monster.name;
    }
    else
    {
        return "None";
    }
}

bool ResourceMonster_Loaded(void)
{
    return currentMonster.loaded;
}

bool ResourceStory_CurrentLocationHasMonster(void)
{
    return currentLocation && currentLocation->monsterCount > 0;
}

int ResourceStory_GetCurrentLocationMonster(void)
{
    if(currentLocation && currentLocation->monsterCount)
    {
        uint16_t index = Random(currentLocation->monsterCount);
        return currentLocation->monsters[index];
    }
    else
    {
        return -1;
    }
}

SkillList *ResourceStory_GetCurrentMonsterSkillList(void)
{
    if(currentMonster.loaded)
        return &currentMonster.monster.skillList;
    else
        return NULL;
}

CombatantClass *ResourceStory_GetCurrentMonsterCombatantClass(void)
{
    if(currentMonster.loaded)
        return &currentMonster.monster.combatantClass;
    else
        return NULL;
}

int ResourceStory_GetCurrentMonsterImage(void)
{
    if(currentMonster.loaded)
    {
        return currentMonster.monster.image;
    }
    else
    {
        return -1;
    }
}


/********************* RESOURCE STORY *******************************/
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
    uint16_t currentLocationIndex;
    uint16_t timeOnPath;
    uint16_t destinationIndex;
} PersistedResourceStoryState;

typedef struct ResourceStoryState
{
    bool needsSaving;
    PersistedResourceStoryState persistedResourceStoryState;
} ResourceStoryState;

static int16_t currentResourceStoryIndex = -1;
static ResourceStoryState currentResourceStoryState = {0};

static ResourceStory **resourceStoryList = NULL;

static ResourceStory *ResourceStory_GetCurrentStory(void)
{
    if(currentResourceStoryIndex < 0)
        return NULL;
    
    return resourceStoryList[currentResourceStoryIndex];
}

static ResHandle ResourceStory_GetCurrentResHandle(void)
{
    if(currentResourceStoryIndex < 0)
        return NULL;
    
    return resource_get_handle(GetStoryResourceIdByIndex(currentResourceStoryIndex));
}

void ResourceStory_InitializeCurrent(void)
{
    currentResourceStoryState.persistedResourceStoryState.currentLocationIndex = ResourceStory_GetCurrentStory()->start_location;
    currentResourceStoryState.persistedResourceStoryState.timeOnPath = 0;

    ResourceStory_UpdateStoryWithPersistedState();
}

uint16_t ResourceStory_GetCurrentLocationIndex(void)
{
    return currentResourceStoryState.persistedResourceStoryState.currentLocationIndex;
}

bool ResourceStory_IncrementTimeOnPath(void)
{
    currentResourceStoryState.persistedResourceStoryState.timeOnPath++;
    return currentResourceStoryState.persistedResourceStoryState.timeOnPath >= currentLocation->length;
}

uint16_t ResourceStory_GetTimeOnPath(void)
{
    return currentResourceStoryState.persistedResourceStoryState.timeOnPath;
}

uint16_t ResourceStory_GetCurrentLocationLength(void)
{
    if(currentLocation)
    {
        return currentLocation->length;
    }
    
    return 0;
}

const char *ResourceStory_GetCurrentLocationName(void)
{
    if(currentLocation)
    {
        return currentLocation->name;
    }
    else
    {
        return "None";
    }
}

ResourceStoryUpdateReturnType ResourceStory_UpdateCurrentLocation(void)
{
    if(ResourceStory_CurrentLocationIsPath())
    {
        bool pathEnded = ResourceStory_IncrementTimeOnPath();
        if(pathEnded)
        {
            return ResourceStory_MoveToLocation(currentResourceStoryState.persistedResourceStoryState.destinationIndex);
        }
        else
        {
            return STORYUPDATE_COMPUTERANDOM;
        }
    }

    return STORYUPDATE_DONOTHING;
}

ResourceStoryUpdateReturnType ResourceStory_MoveToLocation(uint16_t index)
{
    if(currentLocation)
    {
        ResourceLocation *newLocation = adjacentLocations[index];
        adjacentLocations[index] = NULL;

        uint16_t globalIndex = currentLocation->adjacentLocations[index];
        uint16_t oldIndex = currentResourceStoryState.persistedResourceStoryState.currentLocationIndex;
        
        ResourceLocation_FreeAdjacentLocations();
        ResourceLocation_Free(currentLocation);

        currentLocation = newLocation;
        ResourceLocation_LoadAdjacentLocations();
        currentResourceStoryState.persistedResourceStoryState.currentLocationIndex = globalIndex;
        currentResourceStoryState.persistedResourceStoryState.timeOnPath = 0;

        if(ResourceStory_CurrentLocationIsPath())
        {
            if(currentLocation->adjacentLocations[0] == oldIndex)
                currentResourceStoryState.persistedResourceStoryState.destinationIndex = 1;
            else
                currentResourceStoryState.persistedResourceStoryState.destinationIndex = 0;
        }
        else
        {
            if(ResourceStory_CurrentLocationHasMonster())
            {
                TriggerBattleScreen();
                return STORYUPDATE_DONOTHING;
            }
        }
        return STORYUPDATE_FULLREFRESH;
    }
    return STORYUPDATE_DONOTHING;
}

void ResourceStory_FreeAll(void)
{
    for(int i = 0; i < GetStoryCount(); ++i)
    {
        free(resourceStoryList[i]);
    }
    free(resourceStoryList);
}

static ResourceStory *ResourceStory_Load(int resourceId)
{
    ResHandle currentStoryData = resource_get_handle(resourceId);
    ResourceStory *currentResourceStory = calloc(sizeof(ResourceStory), 1);
    ResourceLoadStruct(currentStoryData, 0, (uint8_t*)currentResourceStory, sizeof(ResourceStory), "ResourceStory");
    return currentResourceStory;
}

#if DEBUG_LOGGING > 1
static void ResourceStory_Log(ResourceStory *story)
{
    DEBUG_VERBOSE_LOG("ResourceStory: %s, %s, %d, %d, %d", story->name, story->description, story->id, story->version, story->start_location);
}
#endif

void ResourceStory_LoadAll(void)
{
    if(resourceStoryList)
        return;
    
    resourceStoryList = calloc(sizeof(ResourceStory*), GetStoryCount());
    
    for(int i = 0; i < GetStoryCount(); ++i)
    {
        resourceStoryList[i] = ResourceStory_Load(GetStoryResourceIdByIndex(i));
#if DEBUG_LOGGING > 1
        ResourceStory_Log(resourceStoryList[i]);
#endif
    }
}

void ResourceStory_SetCurrentStory(uint16_t index)
{
    currentResourceStoryIndex = index;
}

void ResourceStory_ClearCurrentStory(void)
{
    currentResourceStoryIndex = -1;
    if(currentLocation)
    {
        ResourceLocation_Free(currentLocation);
        currentLocation = NULL;
        ResourceLocation_FreeAdjacentLocations();
    }
}

void ResourceStory_LogCurrent(void)
{
#if DEBUG_LOGGING
    ResourceStory *currentResourceStory = ResourceStory_GetCurrentStory();
    DEBUG_LOG("ResourceStory: %s, %s, %d, %d, %d", currentResourceStory->name, currentResourceStory->description, currentResourceStory->id, currentResourceStory->version, currentResourceStory->start_location);
#endif
}

const char *ResourceStory_GetNameByIndex(uint16_t index)
{
    if(index >= GetStoryCount())
        return "None";
        
    ResourceStory *story = resourceStoryList[index];
    return story->name;
}

const char *ResourceStory_GetDescriptionByIndex(uint16_t index)
{
    if(index >= GetStoryCount())
        return "None";
    
    ResourceStory *story = resourceStoryList[index];
    return story->description;
}

void ResourceStory_GetStoryList(uint16_t *count, uint16_t **buffer)
{
    *count = GetStoryCount();
    *buffer = calloc(sizeof(uint16_t), *count);
    
    for(int i = 0; i < *count; ++i)
    {
        (*buffer)[i] = resourceStoryList[i]->id;
    }
}

uint16_t ResourceStory_GetCurrentStoryId(void)
{
    return ResourceStory_GetCurrentStory()->id;
}

uint16_t ResourceStory_GetCurrentStoryVersion(void)
{
    return ResourceStory_GetCurrentStory()->version;
}

void ResourceStory_GetPersistedData(uint16_t *count, uint8_t **buffer)
{
    *count = sizeof(currentResourceStoryState.persistedResourceStoryState);
    *buffer = (uint8_t*)(&currentResourceStoryState.persistedResourceStoryState);
}

void ResourceStory_UpdateStoryWithPersistedState(void)
{
    if(currentLocation)
        ResourceLocation_Free(currentLocation);
    
    currentLocation = ResourceLocation_Load(currentResourceStoryState.persistedResourceStoryState.currentLocationIndex);
    ResourceLocation_LoadAdjacentLocations();
#if DEBUG_LOGGING > 1
    ResourceLocation_Log(currentLocation);
#endif
}
