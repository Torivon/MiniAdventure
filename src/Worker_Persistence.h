#pragma once
#include "MiniAdventure.h"

#ifdef BUILD_WORKER_FILES
#include <pebble_worker.h>

typedef struct PersistedResourceStoryState PersistedResourceStoryState;

bool LoadWorkerData(void);
bool SaveWorkerData(void);
bool GetWorkerCanLaunch(void);
bool GetClosedInBattle(void);
PersistedResourceStoryState *GetPersistedStoryState(void);
bool IsCurrentStoryValid(void);
void ForceRandomBattle(void);
#endif
