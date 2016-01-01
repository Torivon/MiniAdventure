#include "pebble.h"

#include "Character.h"
#include "GlobalState.h"
#include "Logging.h"
#include "Monsters.h"
#include "NewBattle.h"
#include "OptionsMenu.h"
#include "Persistence.h"
#include "TitleScreen.h"
#include "WorkerControl.h"
#include "ResourceStory.h"

void ClearStoryPersistedData(uint16_t storyId)
{
    int offset = ComputeStoryPersistedDataOffset(storyId);
    if(persist_exists(PERSISTED_STORY_IS_DATA_SAVED + offset))
    {
        DEBUG_LOG("Clearing game persisted data.");
        int maxKey = persist_read_int(PERSISTED_STORY_MAX_KEY_USED + offset);
        int i;
        for(i = offset; i < maxKey; ++i)
        {
            persist_delete(i);
        }
    }
}

void ClearGlobalPersistedData(void)
{
	if(persist_exists(PERSISTED_IS_DATA_SAVED))
	{
        uint16_t oldcount = 0;
        uint16_t *oldbuffer = NULL;
        oldcount = persist_read_int(PERSISTED_STORY_LIST_SIZE);
        oldbuffer = calloc(sizeof(uint16_t), oldcount);
        persist_read_data(PERSISTED_STORY_LIST, oldbuffer, oldcount);
        
        for(int i = 0; i < oldcount; ++i)
        {
            INFO_LOG("Deleting data for missing story: %d", oldbuffer[i]);
            ClearStoryPersistedData(oldbuffer[i]);
        }
        free(oldbuffer);
        
		int maxKey = persist_read_int(PERSISTED_MAX_KEY_USED);
		int i;
		for(i = 0; i <= maxKey; ++i)
		{
			persist_delete(i);
		}
	}
}

bool SaveGlobalPersistedData(void)
{
    if(!IsGlobalPersistedDataCurrent())
    {
        WARNING_LOG("Persisted data does not match current version, clearing.");
        ClearGlobalPersistedData();
    }

    INFO_LOG("Saving global persisted data.");
    persist_write_bool(PERSISTED_IS_DATA_SAVED, true);
    persist_write_int(PERSISTED_CURRENT_DATA_VERSION, CURRENT_DATA_VERSION);
    persist_write_int(PERSISTED_MAX_KEY_USED, PERSISTED_GLOBAL_DATA_COUNT);
    persist_write_bool(PERSISTED_VIBRATION, GetVibration());
    persist_write_bool(PERSISTED_WORKER_APP, GetWorkerApp());
    persist_write_bool(PERSISTED_WORKER_CAN_LAUNCH, GetWorkerCanLaunch());
    
    uint16_t count = 0;
    uint16_t *buffer = NULL;
    ResourceStory_GetStoryList(&count, &buffer);
    persist_write_int(PERSISTED_STORY_LIST_SIZE, count);
    persist_write_data(PERSISTED_STORY_LIST, buffer, count * sizeof(uint16_t));
    free(buffer);
    
    persist_write_bool(PERSISTED_TUTORIAL_SEEN, GetTutorialSeen());
    
    return true;
}

bool LoadGlobalPersistedData(void)
{
    bool useWorkerApp = false;
    
    if(!persist_exists(PERSISTED_IS_DATA_SAVED) || !persist_read_bool(PERSISTED_IS_DATA_SAVED))
    {
        INFO_LOG("No saved data to load.");
        return false;
    }
    
    if(!IsGlobalPersistedDataCurrent())
    {
        WARNING_LOG("Persisted data does not match current version, clearing.");
        ClearGlobalPersistedData();
        return false;
    }
    
    uint16_t newcount = 0;
    uint16_t *newbuffer = NULL;
    uint16_t oldcount = 0;
    uint16_t *oldbuffer = NULL;
    ResourceStory_GetStoryList(&newcount, &newbuffer);
    oldcount = persist_read_int(PERSISTED_STORY_LIST_SIZE);
    oldbuffer = calloc(sizeof(uint16_t), oldcount);
    persist_read_data(PERSISTED_STORY_LIST, oldbuffer, oldcount * sizeof(uint16_t));

    for(int i = 0; i < oldcount; ++i)
    {
        bool found = false;
        for(int j = 0; j < newcount; ++j)
        {
            if(newbuffer[j] == oldbuffer[i])
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            INFO_LOG("Deleting data for missing story: %d", oldbuffer[i]);
            ClearStoryPersistedData(oldbuffer[i]);
        }
    }
    
    free(oldbuffer);
    free(newbuffer);
    
    INFO_LOG("Loading global persisted data.");
    SetVibration(persist_read_bool(PERSISTED_VIBRATION));
    useWorkerApp = persist_read_bool(PERSISTED_WORKER_APP);
    if(useWorkerApp)
    {
        AttemptToLaunchWorkerApp();
    }
    else
    {
        // If the user has launched the worker app outside of MiniDungeon,
        // they want it on.
        if(WorkerIsRunning())
            SetWorkerApp(true);
    }
    SetWorkerCanLaunch(persist_read_bool(PERSISTED_WORKER_CAN_LAUNCH));
    
    SetTutorialSeen(persist_read_bool(PERSISTED_TUTORIAL_SEEN));
    
    return true;
}

bool SaveStoryPersistedData(void)
{
    uint16_t storyId = ResourceStory_GetCurrentStoryId();
    uint16_t storyVersion = ResourceStory_GetCurrentStoryVersion();
    int offset = ComputeStoryPersistedDataOffset(storyId);

    if(!IsStoryPersistedDataCurrent(storyId, storyVersion))
    {
        WARNING_LOG("Persisted data does not match current version, clearing.");
        ClearStoryPersistedData(storyId);
    }
    
    INFO_LOG("Saving story persisted data.");
    persist_write_bool(offset + PERSISTED_STORY_IS_DATA_SAVED, true);
    persist_write_int(offset + PERSISTED_STORY_CURRENT_DATA_VERSION, storyVersion);
    persist_write_int(offset + PERSISTED_STORY_MAX_KEY_USED, offset + PERSISTED_STORY_DATA_COUNT);
    uint16_t count;
    uint8_t *buffer;
    ResourceStory_GetPersistedData(&count, &buffer);
    persist_write_data(offset + PERSISTED_STORY_STORY_DATA, buffer, count);
    
    return true;
}

bool LoadStoryPersistedData(void)
{
    uint16_t storyId = ResourceStory_GetCurrentStoryId();
    uint16_t storyVersion = ResourceStory_GetCurrentStoryVersion();
    int offset = ComputeStoryPersistedDataOffset(storyId);
    
    if(!persist_exists(offset + PERSISTED_STORY_IS_DATA_SAVED) || !persist_read_bool(offset + PERSISTED_STORY_IS_DATA_SAVED))
    {
        INFO_LOG("No saved data to load.");
        return false;
    }
    
    if(!IsStoryPersistedDataCurrent(storyId, storyVersion))
    {
        WARNING_LOG("Persisted data does not match current version, clearing.");
        ClearStoryPersistedData(storyId);
        return false;
    }
    
    uint16_t count;
    uint8_t *buffer;
    ResourceStory_GetPersistedData(&count, &buffer);
    persist_read_data(offset + PERSISTED_STORY_STORY_DATA, buffer, count);
    
    ResourceStory_UpdateStoryWithPersistedState();
    
    return true;
}
