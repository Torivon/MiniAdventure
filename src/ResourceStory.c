#include "pebble.h"

#include "BinaryResourceLoading.h"
#include "Utils.h"
#include "Location.h"
#include "Logging.h"
#include "Monsters.h"
#include "ResourceStory.h"
#include "AutoImageMap.h"

static ResourceStory *currentResourceStory = NULL;
static ResHandle currentStoryData = {0};
static ResourceStoryState currentResourceStoryState = {0};
static ResourceLocation *currentLocation = NULL;


void ResourceStory_InitializeCurrent(void)
{
    currentResourceStoryState.persistedResourceStoryState.currentLocationIndex = currentResourceStory->start_location;
    
    if(currentLocation)
        ResourceLocation_Free(currentLocation);
    
    DEBUG_VERBOSE_LOG("New location logical index: %d", currentResourceStoryState.persistedResourceStoryState.currentLocationIndex);
    currentLocation = ResourceLocation_Load(currentResourceStoryState.persistedResourceStoryState.currentLocationIndex);
    ResourceLocation_Log(currentLocation);
}

uint16_t ResourceStory_GetCurrentLocationIndex(void)
{
    return currentResourceStoryState.persistedResourceStoryState.currentLocationIndex;
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

uint16_t ResourceStory_GetCurrentAdjacentLocations(void)
{
    if(currentLocation)
        return currentLocation->adjacentLocationCount;
    else
        return 0;
}

void ResourceStory_MoveToLocation(uint16_t index)
{
    if(currentLocation)
    {
        ResourceLocation *newLocation = ResourceLocation_Load(currentLocation->adjacentLocations[index]);
        ResourceLocation_Free(currentLocation);
        currentLocation = newLocation;
        currentResourceStoryState.persistedResourceStoryState.currentLocationIndex = currentLocation->adjacentLocations[index];
    }
}

void ResourceStory_FreeCurrent(void)
{
    if(currentResourceStory)
    {
        free(currentResourceStory);
        currentResourceStory = NULL;
        if(currentLocation)
        {
            ResourceLocation_Free(currentLocation);
            currentLocation = NULL;
        }
    }
}

void ResourceStory_Load(int resourceId)
{
    ResourceStory_FreeCurrent();
    currentStoryData = resource_get_handle(resourceId);
    
    currentResourceStory = calloc(sizeof(ResourceStory), 1);
 
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
}

void ResourceStory_LogCurrent(void)
{
    DEBUG_LOG("ResourceStory: %s, %s, %d, %d, %d", currentResourceStory->name, currentResourceStory->description, currentResourceStory->id, currentResourceStory->version, currentResourceStory->start_location);
}

void ResourceLocation_Free(ResourceLocation *location)
{
    if(location)
    {
        free(location);
    }
}

ResourceLocation *ResourceLocation_Load(uint16_t index)
{
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
    return newLocation;
}

void ResourceLocation_Log(ResourceLocation *location)
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

int ResourceStory_GetCurrentLocationBackgroundImageId(void)
{
    //TODO: Add randomness
    if(currentLocation && currentLocation->backgroundImageCount)
    {
        return currentLocation->backgroundImages[0];
    }
    else
    {
        return -1;
    }
}


