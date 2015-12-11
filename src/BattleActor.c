#include <pebble.h>
#include "BattleActor.h"

typedef struct BattleActor
{
	bool isPlayer;
	uint16_t level;
	uint16_t speed;
	uint16_t health;
	uint16_t maxHealth;
} BattleActor;

BattleActor player = {0};
BattleActor monster = {0};

uint16_t BattleActor_GetSpeed(BattleActor *actor)
{
	return actor->speed;
}

bool BattleActor_IsPlayer(BattleActor *actor)
{
	return actor->isPlayer;
}

BattleActor *InitBattleActor(bool isPlayer, uint16_t level, uint16_t speed, uint16_t maxHealth)
{
	BattleActor *returnValue = isPlayer ? &player : &monster;
	returnValue->isPlayer = isPlayer;
	returnValue->level = level;
	returnValue->speed = speed;
	returnValue->maxHealth = returnValue->health = maxHealth;
	return returnValue;
}

void DealDamage(uint16_t potency, BattleActor *defender)
{
	if(!defender)
		return;
	defender->health -= potency;
}

uint16_t BattleActor_GetHealth(BattleActor *actor)
{
	if(!actor)
		return 0;
	
	return actor->health;
}

uint16_t BattleActor_GetMaxHealth(BattleActor *actor)
{
	if(!actor)
		return 0;
	
	return actor->maxHealth;
}
