#pragma once

typedef struct BattleActor BattleActor;
	
uint16_t BattleActor_GetSpeed(BattleActor *actor);
bool BattleActor_IsPlayer(BattleActor *actor);

BattleActor *InitBattleActor(bool isPlayer, uint16_t level, uint16_t speed, uint16_t maxHealth);
void DealDamage(uint16_t potency, BattleActor *defender);

uint16_t BattleActor_GetHealth(BattleActor *actor);
uint16_t BattleActor_GetMaxHealth(BattleActor *actor);
