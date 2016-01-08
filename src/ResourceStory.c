#include "pebble.h"

#include "AutoImageMap.h"
#include "AutoSizeConstants.h"
#include "AutoLocationConstants.h"
#include "BinaryResourceLoading.h"
#include "Character.h"
#include "CombatantClass.h"
#include "Utils.h"
#include "Logging.h"
#include "Battle.h"
#include "ResourceStory.h"
#include "Skills.h"
#include "StoryList.h"


/********************* PREDECLARATIONS *********************************/

typedef struct ResourceStory
{
    uint16_t id;
    uint16_t version;
    uint16_t hash;
    char name[MAX_STORY_NAME_LENGTH];
    char description[MAX_STORY_DESC_LENGTH];
    uint16_t start_location;
    uint16_t xpMonstersPerLevel;
    uint16_t xpDifferenceScale;
    uint16_t classCount;
    uint16_t classes[MAX_CLASSES];
} ResourceStory;


static ResHandle ResourceStory_GetCurrentResHandle(void);
static ResourceStory *ResourceStory_GetCurrentStory();

/********************* RESOURCE LOCATION *******************************/
typedef struct ResourceLocation
{
    char name[MAX_STORY_NAME_LENGTH];
    uint16_t adjacentLocationCount;
    uint16_t adjacentLocations[MAX_ADJACENT_LOCATIONS];
    uint16_t backgroundImageCount;
    uint16_t backgroundImages[MAX_BACKGROUND_IMAGES];
    uint16_t locationProperties;
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

bool ResourceStory_CurrentLocationIsRestArea(void)
{
    if(currentLocation)
    {
        return currentLocation->locationProperties & LOCATION_PROPERTY_REST_AREA;
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

/********************* RESOURCE BATTLER ******************************/

BattlerWrapper currentMonster = {0};

BattlerWrapper playerClass = {0};

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

CombatantClass *BattlerWrapper_GetCombatantClass(BattlerWrapper *wrapper)
{
    return &wrapper->battler.combatantClass;
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

void ResourceBattler_UnloadBattler(BattlerWrapper *wrapper)
{
    if(!wrapper->loaded)
        return;
    
    wrapper->loaded = false;
    for(int i = 0; i < wrapper->battler.skillList.count; ++i)
    {
        ResourceSkill_Free(wrapper->loadedSkills[i]);
        wrapper->loadedSkills[i] = NULL;
    }
}

void ResourceMonster_UnloadCurrent(void)
{
    ResourceBattler_UnloadBattler(&currentMonster);
}

void ResourceBattler_UnloadPlayer(void)
{
    ResourceBattler_UnloadBattler(&playerClass);
}

void ResourceBattler_LoadBattler(BattlerWrapper *wrapper, uint16_t logical_index)
{
    ResHandle currentStoryData = ResourceStory_GetCurrentResHandle();
    ResourceLoadStruct(currentStoryData, logical_index, (uint8_t*)(&(wrapper->battler)), sizeof(ResourceBattler), "ResourceBattler");
    
    wrapper->battler.image = autoImageMap[wrapper->battler.image];
    for(int i = 0; i < wrapper->battler.skillList.count; ++i)
    {
        wrapper->loadedSkills[i] = ResourceSkill_Load(wrapper->battler.skillList.entries[i].id);
        wrapper->battler.skillList.entries[i].id = i;
    }
    wrapper->loaded = true;
}

void ResourceMonster_LoadCurrent(uint16_t logical_index)
{
    ResourceBattler_LoadBattler(&currentMonster, logical_index);
}

void ResourceBattler_LoadPlayer(uint16_t classId)
{
    ResourceStory *currentStory = ResourceStory_GetCurrentStory();
    if(!currentStory)
        return;
    
    ResourceBattler_LoadBattler(&playerClass, currentStory->classes[classId]);
}

char *ResourceMonster_GetCurrentName(void)
{
    if(currentMonster.loaded)
    {
        return currentMonster.battler.name;
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

/********************* RESOURCE STORY *******************************/
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

        if(currentLocation && (currentLocation->locationProperties & LOCATION_PROPERTY_REST_AREA))
        {
            Character_Rest();
        }
        
        if(currentLocation && (currentLocation->locationProperties & LOCATION_PROPERTY_LEVEL_UP))
        {
            Character_GrantLevel();
        }

        if(currentLocation && (currentLocation->locationProperties & LOCATION_PROPERTY_GAME_WIN))
        {
            return STORYUPDATE_WIN;
        }

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

bool ResourceStory_InStory(void)
{
    return currentResourceStoryIndex != -1;
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

uint16_t ResourceStory_GetCurrentStoryHash(void)
{
    return ResourceStory_GetCurrentStory()->hash;
}

uint16_t ResourceStory_GetCurrentStoryXPMonstersPerLevel(void)
{
    return ResourceStory_GetCurrentStory()->xpMonstersPerLevel;
}

uint16_t ResourceStory_GetCurrentStoryXPDifferenceScale(void)
{
    return ResourceStory_GetCurrentStory()->xpDifferenceScale;
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
