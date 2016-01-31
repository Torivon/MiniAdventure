#pragma once
#include "MiniAdventure.h"

#ifdef BUILD_WORKER_FILES
#include <pebble_worker.h>

typedef struct PersistedStoryState PersistedStoryState;

bool LoadWorkerData(void);
bool SaveWorkerData(void);
bool GetWorkerCanLaunch(void);
bool GetClosedInBattle(void);
PersistedStoryState *GetPersistedStoryState(void);
bool IsCurrentStoryValid(void);
void ForceRandomBattle(void);
#endif
