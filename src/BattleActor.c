#include <pebble.h>
#include "BattleActor.h"

typedef struct BattleActor
{
	bool isPlayer;
	int level;
	int speed;
	int health;
	int maxHealth;
} BattleActor;

BattleActor player = {0};
BattleActor monster = {0};

int BattleActor_GetSpeed(BattleActor *actor)
{
	return actor->speed;
}

bool BattleActor_IsPlayer(BattleActor *actor)
{
	return actor->isPlayer;
}

BattleActor *InitBattleActor(bool isPlayer, int level, int speed, int maxHealth)
{
	BattleActor *returnValue = isPlayer ? &player : &monster;
	returnValue->isPlayer = isPlayer;
	returnValue->level = level;
	returnValue->speed = speed;
	returnValue->maxHealth = returnValue->health = maxHealth;
	return returnValue;
}

void DealDamage(int potency, BattleActor *defender)
{
	if(!defender)
		return;
	defender->health -= potency;
}

int BattleActor_GetHealth(BattleActor *actor)
{
	if(!actor)
		return 0;
	
	return actor->health;
}

int BattleActor_GetMaxHealth(BattleActor *actor)
{
	if(!actor)
		return 0;
	
	return actor->maxHealth;
}
