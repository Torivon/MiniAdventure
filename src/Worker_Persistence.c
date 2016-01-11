#include "MiniAdventure.h"
#ifdef BUILD_WORKER_FILES
#include <pebble_worker.h>

#include "../src/Persistence.h"
#include "Worker_Persistence.h"
#include "../src/ResourceStory.h"

#if ALLOW_WORKER_APP
static bool closedInBattle = false;
static bool workerCanLaunch = false;
PersistedResourceStoryState persistedState = {0};
static bool currentStoryValid = false;
static int currentStoryId = 0;
static bool forceRandomBattle = false;

bool GetClosedInBattle(void)
{
	return closedInBattle;
}

bool GetWorkerCanLaunch(void)
{
	return workerCanLaunch;
}

bool IsCurrentStoryValid(void)
{
    return currentStoryValid;
}

void ForceRandomBattle(void)
{
    forceRandomBattle = true;
}

PersistedResourceStoryState *GetPersistedStoryState(void)
{
    return &persistedState;
}

bool LoadWorkerData(void)
{
	if(!persist_exists(PERSISTED_IS_DATA_SAVED) || !persist_read_bool(PERSISTED_IS_DATA_SAVED))
		return false;
		
	if(!IsGlobalPersistedDataCurrent())
	{
		return false;
	}

	workerCanLaunch = persist_read_bool(PERSISTED_WORKER_CAN_LAUNCH);
    
    currentStoryValid = persist_read_bool(PERSISTED_CURRENT_GAME_VALID);
    
    if(currentStoryValid)
    {
        currentStoryId = persist_read_int(PERSISTED_CURRENT_GAME);
        int offset = ComputeStoryPersistedDataOffset(currentStoryId);
        if(!persist_exists(offset + PERSISTED_STORY_IS_DATA_SAVED) || !persist_read_bool(offset + PERSISTED_STORY_IS_DATA_SAVED))
        {
            currentStoryValid = false;
            return false;
        }
        
        closedInBattle = persist_read_bool(offset + PERSISTED_STORY_IN_COMBAT);
        persist_read_data(offset + PERSISTED_STORY_STORY_DATA, (uint8_t*)&persistedState, sizeof(PersistedResourceStoryState));

        forceRandomBattle = persist_read_bool(offset + PERSISTED_STORY_FORCE_RANDOM_BATTLE);
    }
    
	return true;
}

bool SaveWorkerData(void)
{
    if(currentStoryValid)
    {
        int offset = ComputeStoryPersistedDataOffset(currentStoryId);
        persist_write_data(offset + PERSISTED_STORY_STORY_DATA, (uint8_t*)&persistedState, sizeof(PersistedResourceStoryState));
        persist_write_bool(offset + PERSISTED_STORY_FORCE_RANDOM_BATTLE, forceRandomBattle);
    }
    
    return true;
}

#endif
#endif
