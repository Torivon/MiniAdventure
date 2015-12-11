#include <pebble.h>
#include "BattleActor.h"
#include "BattleQueue.h"
#include "Logging.h"
#include "MiniAdventure.h"
#include "NewBattle.h"
#include "Skills.h"

#define UP true
#define DOWN false

typedef struct BattleQueueEntry
{
	uint16_t currentTime;
	BattleQueueEntryType type;
	void *data;
	bool active;
	uint16_t index;
	BattleActor *attacker;
	BattleActor *defender;
} BattleQueueEntry;

// priority queue. Probably build a heap.
typedef struct BattleQueue	
{
	BattleQueueEntry *entries[MAX_BATTLE_QUEUE];
	uint16_t count;
} BattleQueue;

static BattleQueue queue = {.entries = {0}, .count = 0};
static BattleQueueEntry globalEntries[MAX_BATTLE_QUEUE];

static void BattleQueueSwap(uint16_t i, uint16_t j)
{
	BattleQueueEntry *temp = queue.entries[i];
	queue.entries[i] = queue.entries[j];
	queue.entries[j] = temp;
	queue.entries[i]->index = i;
	queue.entries[j]->index = j;
}

static uint16_t GetBattleQueueEntrySpeed(BattleQueueEntry *entry)
{
	switch(entry->type)
	{
		case SKILL:
		{
			return GetSkillSpeed((Skill*)entry->data);
		}
		case ACTOR:
		{
			return BattleActor_GetSpeed((BattleActor*)entry->data);
		}
	}
	return 100;
}

static uint16_t GetTicksToAct(BattleQueueEntry *entry, uint16_t *tieBreaker)
{
	uint16_t speed = GetBattleQueueEntrySpeed(entry);
	
	uint16_t ticksToAct = 0;
	uint16_t tempTime = entry->currentTime;
	while(tempTime < TIME_TO_ACT)
	{
		tempTime += speed;
		++ticksToAct;
	}
	
	if(tieBreaker)
		*tieBreaker = tempTime - TIME_TO_ACT;
	
	return ticksToAct;
}

static bool BattleQueueCompare(uint16_t lhs, uint16_t rhs, bool up)
{
	if(up)
		return lhs < rhs;
	else
		return rhs < lhs;
}

static bool BubbleLoopCondition(uint16_t index, bool up)
{
	if(up)
	{
		return index > 0;
	}
	else
	{
		return index < queue.count - 1;
	}
}

static void Bubble(uint16_t start, bool up)
{
	uint16_t index = start;
	int direction = up ? -1 : 1;
	while(BubbleLoopCondition(index, up))
	{
		if(BattleQueueCompare(GetTicksToAct(queue.entries[index], NULL), GetTicksToAct(queue.entries[index + direction], NULL), up))
		{
			BattleQueueSwap(index, index + direction);
			index = index + direction;
		}
		else
		{
			return;
		}
	}
}

static BattleQueueEntry *GetNextInactiveEntry(void)
{
	for(int i = 0; i < MAX_BATTLE_QUEUE; ++i)
	{
		if(!globalEntries[i].active)
		{
			globalEntries[i].active = true;
			return &globalEntries[i];
		}
	}
	return NULL;
}

static bool BattleQueuePush_internal(BattleQueueEntryType type, void *data, BattleActor *attacker, BattleActor *defender)
{
	if(queue.count >= MAX_BATTLE_QUEUE)
		return false;

	BattleQueueEntry *entry = GetNextInactiveEntry();
	if(!entry)
	{
		ERROR_LOG("Too many battle actions");
		return false;
	}
		
	entry->type = type;
	entry->data = data;
	entry->currentTime = 0;
	entry->active = true;
	entry->attacker = attacker;
	entry->defender = defender;
	
	queue.entries[queue.count] = entry;
	entry->index = queue.count;
	++queue.count;
	
	Bubble(queue.count - 1, UP);
	return true;
}

bool BattleQueuePush(BattleQueueEntryType type, void *data, BattleActor *attacker, BattleActor *defender)
{
	DEBUG_LOG("Push type: %u", type);
	bool success = true;
	success = success && BattleQueuePush_internal(type, data, attacker, defender);
	// SO BAD
	if(type == SKILL)
		success = success && BattleQueuePush_internal(ACTOR, attacker, NULL, NULL);
	return success;
}

static void BattleQueueRemove(uint16_t index)
{
	DEBUG_LOG("Remove index %u", index);
	BattleQueueSwap(index, queue.count - 1);
	BattleQueueEntry *old = queue.entries[queue.count - 1];
	old->active = false;
	--queue.count;
		
	Bubble(index, DOWN);
}

static void UpdateBattleQueueEntry(BattleQueueEntry *entry)
{
	entry->currentTime += GetBattleQueueEntrySpeed(entry);
	Bubble(entry->index, UP);
	Bubble(entry->index, DOWN);
}

void LogBattleQueue(void)
{
	DEBUG_VERBOSE_LOG("Queue state");
	for(int i = 0; i < queue.count; ++i)
	{
		BattleQueueEntry *entry = queue.entries[i];
		switch(entry->type)
		{
			case SKILL:
			{
				DEBUG_VERBOSE_LOG("Skill: %s, time: %u, speed: %u, active %s", GetSkillName((Skill*)entry->data), entry->currentTime, GetSkillSpeed((Skill*)entry->data), entry->active ? "true" : "false");
				break;
			}
			case ACTOR:
			{
				DEBUG_VERBOSE_LOG("Actor: %s, time: %u, speed: %u, active %s", BattleActor_IsPlayer((BattleActor*)entry->data) ? "Player" : "Monster", entry->currentTime, BattleActor_GetSpeed((BattleActor*)entry->data), entry->active ? "true" : "false");
				break;
			}
		}
	}
}

bool UpdateBattleQueue(void)
{
	for(int i = 0; i < queue.count; ++i)
	{
		UpdateBattleQueueEntry(queue.entries[i]);
	}
	
	LogBattleQueue();
	
	while(queue.count > 0 && queue.entries[0]->currentTime >= TIME_TO_ACT)
	{
		BattleQueueEntry *top = queue.entries[0];
		switch(top->type)
		{
			case SKILL:
			{
				ExecuteSkill((Skill*)top->data, top->attacker, top->defender);
				break;
			}
			case ACTOR:
			{
				if(BattleActor_IsPlayer((BattleActor*)top->data))
				{
					BattleQueueRemove(0);
					return true;	
				}
				else
				{
					BattleQueuePush(SKILL, GetFastAttack(), GetMonsterActor(), GetPlayerActor());
				}
				break;
			}
		}
		
		BattleQueueRemove(0);
	}
	return false;
}
