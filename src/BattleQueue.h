#pragma once

typedef struct BattleActor BattleActor;

typedef enum 
{
	ACTOR = 0,
	SKILL,
} BattleQueueEntryType;

bool UpdateBattleQueue(BattleQueueEntryType *type, void **data);
bool BattleQueuePush(BattleQueueEntryType type, void *data);
void ResetBattleQueue(void);
int GetCurrentTimeInQueue(bool player);

