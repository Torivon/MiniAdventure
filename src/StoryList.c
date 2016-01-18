#include "pebble.h"
#include "StoryList.h"
#include "AutoStoryList.h"

int GetStoryCount(void)
{
    return sizeof(autoStoryList)/sizeof(*autoStoryList);
}

int GetStoryResourceIdByIndex(int index)
{
    if(index >= GetStoryCount())
        return -1;
    else
        return autoStoryList[index];
}
