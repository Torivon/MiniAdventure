#pragma once

#define CURRENT_DATA_VERSION 1

#define PERSISTED_DATA_GAP 1000

enum
{
	PERSISTED_IS_DATA_SAVED = 0,
	PERSISTED_CURRENT_DATA_VERSION,
	PERSISTED_MAX_KEY_USED,
	PERSISTED_VIBRATION,
	PERSISTED_WORKER_APP,
	PERSISTED_WORKER_CAN_LAUNCH,
	PERSISTED_CURRENT_GAME,
    PERSISTED_STORY_LIST_SIZE,
    PERSISTED_STORY_LIST,
    PERSISTED_TUTORIAL_SEEN,
	PERSISTED_GLOBAL_DATA_COUNT,
};

enum
{
	PERSISTED_STORY_IS_DATA_SAVED = 0,
	PERSISTED_STORY_CURRENT_DATA_VERSION,
    PERSISTED_STORY_MAX_KEY_USED,
	PERSISTED_STORY_CHARACTER_DATA,
	PERSISTED_STORY_STORY_DATA,
	
	PERSISTED_STORY_IN_COMBAT,
	PERSISTED_STORY_MONSTER_TYPE,
	PERSISTED_STORY_MONSTER_HEALTH,
	
	// This needs to always be last
	PERSISTED_STORY_DATA_COUNT
};

inline bool IsGlobalPersistedDataCurrent(void)
{
	bool dataSaved = persist_read_bool(PERSISTED_IS_DATA_SAVED);
	int savedVersion;
	if(!dataSaved)
		return true;
	
	savedVersion = persist_read_int(PERSISTED_CURRENT_DATA_VERSION);
	
	return savedVersion == CURRENT_DATA_VERSION;
}

inline int ComputeStoryPersistedDataOffset(uint16_t storyId)
{
	return PERSISTED_DATA_GAP * storyId;
}
	
inline bool IsStoryPersistedDataCurrent(uint16_t storyId, uint16_t storyDataVersion)
{
	int offset = ComputeStoryPersistedDataOffset(storyId);
	bool dataSaved = persist_read_bool(offset + PERSISTED_STORY_IS_DATA_SAVED);
	int savedVersion;
	if(!dataSaved)
		return true;
	
	savedVersion = persist_read_int(offset + PERSISTED_STORY_CURRENT_DATA_VERSION);
	
	return savedVersion == storyDataVersion;
}

bool SaveGlobalPersistedData(void);
bool LoadGlobalPersistedData(void);

bool SaveStoryPersistedData(void);
bool LoadStoryPersistedData(void);
