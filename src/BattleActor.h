#pragma once

typedef struct BattleActor BattleActor;
typedef struct CombatantClass CombatantClass;
typedef struct SkillList SkillList;

int BattleActor_GetSpeed(BattleActor *actor);
bool BattleActor_IsPlayer(BattleActor *actor);
void BattleActor_SetCurrentTime(BattleActor *actor, int currentTime);

BattleActor *InitBattleActor(bool isPlayer, CombatantClass *combatantClass, SkillList *skillList, int level);
void DealDamage(int potency, BattleActor *defender);

int BattleActor_GetHealth(BattleActor *actor);
int BattleActor_GetMaxHealth(BattleActor *actor);

SkillList *BattleActor_GetSkillList(BattleActor *actor);
CombatantClass *BattleActor_GetCombatantClass(BattleActor *actor);
