#pragma once

typedef struct BattleActor BattleActor;

typedef enum 
{
	ACTOR = 0,
	SKILL,
} BattleQueueEntryType;

bool UpdateBattleQueue(void);
bool BattleQueuePush(BattleQueueEntryType type, void *data, BattleActor *attacker, BattleActor *defender);
