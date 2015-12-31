#include "pebble.h"

#include "BinaryResourceLoading.h"
#include "Utils.h"
#include "Location.h"
#include "Logging.h"
#include "Monsters.h"
#include "ResourceStory.h"

static ResourceStory *currentResourceStory = NULL;
//static ResourceStoryState *currentResourceStoryState = NULL;

void ResourceStory_FreeCurrent(void)
{
    if(currentResourceStory)
    {
        free(currentResourceStory);
        currentResourceStory = NULL;
    }
}
void ResourceStory_Load(int resourceId)
{
    ResourceStory_FreeCurrent();
    ResHandle storyData = resource_get_handle(resourceId);
    
    currentResourceStory = calloc(sizeof(ResourceStory), 1);
    uint8_t buffer[256] = "";
 
    int location_index = 2;
    int read_index = ResourceLoad16BitInt(storyData, location_index);
    currentResourceStory->id = ResourceLoad16BitInt(storyData, read_index);
    read_index += 2;
    currentResourceStory->version = ResourceLoad16BitInt(storyData, read_index);
    read_index += 2;
    int string_length = ResourceLoad16BitInt(storyData, read_index);
    read_index += 2;
    ResourceLoadString(storyData, read_index, currentResourceStory->name, string_length);
    read_index += string_length;
    string_length = ResourceLoad16BitInt(storyData, read_index);
    read_index += 2;
    ResourceLoadString(storyData, read_index, currentResourceStory->description, string_length);
}

void ResourceStory_LogCurrent(void)
{
    DEBUG_VERBOSE_LOG("ResourceStory: %s, %s, %d, %d", currentResourceStory->name, currentResourceStory->description, currentResourceStory->id, currentResourceStory->version);
}
