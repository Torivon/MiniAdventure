#pragma once

typedef struct BattleActor BattleActor;
	
int BattleActor_GetSpeed(BattleActor *actor);
bool BattleActor_IsPlayer(BattleActor *actor);
void BattleActor_SetCurrentTime(BattleActor *actor, int currentTime);

BattleActor *InitBattleActor(bool isPlayer, int level, int speed, int maxHealth);
void DealDamage(int potency, BattleActor *defender);

int BattleActor_GetHealth(BattleActor *actor);
int BattleActor_GetMaxHealth(BattleActor *actor);
