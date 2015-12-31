#include "pebble.h"

#include "AutoImageMap.h"
#include "AutoSizeConstants.h"
#include "BinaryResourceLoading.h"
#include "Utils.h"
#include "Location.h"
#include "Logging.h"
#include "Monsters.h"
#include "ResourceStory.h"
#include "StoryList.h"

typedef struct ResourceLocation
{
    char name[MAX_STORY_NAME_LENGTH];
    uint16_t adjacentLocationCount;
    uint16_t adjacentLocations[MAX_ADJACENT_LOCATIONS];
    uint16_t backgroundImageCount;
    uint16_t backgroundImages[MAX_BACKGROUND_IMAGES];
    uint16_t length;
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
static ResourceLocation *currentLocation = NULL;
static ResourceLocation *adjacentLocations[MAX_ADJACENT_LOCATIONS] = {0};

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

static ResourceLocation *ResourceLocation_Load(uint16_t index)
{
    ResHandle currentStoryData = ResourceStory_GetCurrentResHandle();
    ResourceLocation *newLocation = calloc(sizeof(ResourceLocation), 1);
    DEBUG_VERBOSE_LOG("Logical index: %d", index);
    
    int start_index = ResourceLoad_GetByteIndexFromLogicalIndex(index);
    int read_index = ResourceLoad16BitInt(currentStoryData, &start_index);
    DEBUG_VERBOSE_LOG("Location start index: %d", read_index);
    int string_length = ResourceLoad16BitInt(currentStoryData, &read_index);
    ResourceLoadString(currentStoryData, &read_index, newLocation->name, string_length);
    newLocation->adjacentLocationCount = ResourceLoad16BitInt(currentStoryData, &read_index);
    for(int i = 0; i < newLocation->adjacentLocationCount; ++i)
    {
        newLocation->adjacentLocations[i] = ResourceLoad16BitInt(currentStoryData, &read_index);
    }
    newLocation->backgroundImageCount = ResourceLoad16BitInt(currentStoryData, &read_index);
    for(int i = 0; i < newLocation->backgroundImageCount; ++i)
    {
        newLocation->backgroundImages[i] = autoImageMap[ResourceLoad16BitInt(currentStoryData, &read_index)];
    }
    
    newLocation->length = ResourceLoad16BitInt(currentStoryData, &read_index);
    
    return newLocation;
}

static void ResourceLocation_Free(ResourceLocation *location)
{
    if(location)
    {
        free(location);
    }
}

static void ResourceLocation_Log(ResourceLocation *location)
{
    DEBUG_LOG("ResourceLocation: %s", location->name);
    for(int i = 0; i < location->adjacentLocationCount; ++i)
    {
        DEBUG_LOG("Adjacent Location: %d", location->adjacentLocations[i]);
    }
    for(int i = 0; i < location->backgroundImageCount; ++i)
    {
        DEBUG_LOG("Background: %d", location->backgroundImages[i]);
    }
}

static void ResourceLocation_LoadAdjacentLocations(void)
{
    for(int i = 0; i < currentLocation->adjacentLocationCount; ++i)
    {
        adjacentLocations[i] = ResourceLocation_Load(currentLocation->adjacentLocations[i]);
    }
}

void ResourceStory_InitializeCurrent(void)
{
    currentResourceStoryState.persistedResourceStoryState.currentLocationIndex = ResourceStory_GetCurrentStory()->start_location;
    currentResourceStoryState.persistedResourceStoryState.timeOnPath = 0;
    
    if(currentLocation)
        ResourceLocation_Free(currentLocation);
    
    DEBUG_VERBOSE_LOG("New location logical index: %d", currentResourceStoryState.persistedResourceStoryState.currentLocationIndex);
    currentLocation = ResourceLocation_Load(currentResourceStoryState.persistedResourceStoryState.currentLocationIndex);
    ResourceLocation_LoadAdjacentLocations();
    ResourceLocation_Log(currentLocation);
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

bool ResourceStory_CurrentLocationIsPath(void)
{
    if(currentLocation)
    {
        return currentLocation->length > 0;
    }
    
    return false;
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
            ResourceStory_MoveToLocation(currentResourceStoryState.persistedResourceStoryState.destinationIndex);
            return STORYUPDATE_FULLREFRESH;
        }
        else
        {
            return STORYUPDATE_COMPUTERANDOM;
        }
    }

    return STORYUPDATE_DONOTHING;
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

void ResourceStory_MoveToLocation(uint16_t index)
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
            DEBUG_VERBOSE_LOG("current: %d, adjacent[0]: %d, adjacent[1]: %d", oldIndex, currentLocation->adjacentLocations[0], currentLocation->adjacentLocations[1]);
            if(currentLocation->adjacentLocations[0] == oldIndex)
                currentResourceStoryState.persistedResourceStoryState.destinationIndex = 1;
            else
                currentResourceStoryState.persistedResourceStoryState.destinationIndex = 0;
        }
    }
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
 
    // The story is always written out first, so its location is just after count
    int start_index = 0;
    int count = ResourceLoad16BitInt(currentStoryData, &start_index);
    DEBUG_VERBOSE_LOG("Count: %d", count);
    int read_index = ResourceLoad16BitInt(currentStoryData, &start_index);
    DEBUG_VERBOSE_LOG("Index for story: %d", read_index);
    currentResourceStory->id = ResourceLoad16BitInt(currentStoryData, &read_index);
    currentResourceStory->version = ResourceLoad16BitInt(currentStoryData, &read_index);
    int string_length = ResourceLoad16BitInt(currentStoryData, &read_index);
    ResourceLoadString(currentStoryData, &read_index, currentResourceStory->name, string_length);
    string_length = ResourceLoad16BitInt(currentStoryData, &read_index);
    ResourceLoadString(currentStoryData, &read_index, currentResourceStory->description, string_length);
    currentResourceStory->start_location = ResourceLoad16BitInt(currentStoryData, &read_index);
    
    return currentResourceStory;
}

void ResourceStory_LoadAll(void)
{
    if(resourceStoryList)
        return;
    
    resourceStoryList = calloc(sizeof(ResourceStory*), GetStoryCount());
    
    for(int i = 0; i < GetStoryCount(); ++i)
    {
        resourceStoryList[i] = ResourceStory_Load(GetStoryResourceIdByIndex(i));
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
    ResourceStory *currentResourceStory = ResourceStory_GetCurrentStory();
    DEBUG_LOG("ResourceStory: %s, %s, %d, %d, %d", currentResourceStory->name, currentResourceStory->description, currentResourceStory->id, currentResourceStory->version, currentResourceStory->start_location);
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

