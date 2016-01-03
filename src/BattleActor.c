#include <pebble.h>
#include "BattleActor.h"
#include "CombatantClass.h"
#include "Logging.h"
#include "Skills.h"

typedef struct BattleActor
{
    bool isPlayer;
    CombatantClass *combatantClass;
    SkillList *skillList;
    int level;
    int speed;
    int health;
    int maxHealth;
    int currentTime;
    SkillInstance *counter;
} BattleActor;

BattleActor player = {0};
BattleActor monster = {0};

SkillInstance *BattleActor_GetCounter(BattleActor *actor)
{
    return actor->counter;
}

void BattleActor_SetCounter(BattleActor *actor, SkillInstance *counter)
{
    actor->counter = counter;
}

int BattleActor_GetSpeed(BattleActor *actor)
{
    return actor->speed;
}

bool BattleActor_IsPlayer(BattleActor *actor)
{
    return actor->isPlayer;
}

int BattleActor_GetLevel(BattleActor *actor)
{
    return actor->level;
}

void BattleActor_SetCurrentTime(BattleActor *actor, int currentTime)
{
    actor->currentTime = currentTime;
}

SkillList *BattleActor_GetSkillList(BattleActor *actor)
{
    return actor->skillList;
}

CombatantClass *BattleActor_GetCombatantClass(BattleActor *actor)
{
    return actor->combatantClass;
}

BattleActor *BattleActor_Init(bool isPlayer, CombatantClass *combatantClass, SkillList *skillList, int level, int startingHealth)
{
    DEBUG_VERBOSE_LOG("BattleActor_Init: %s", isPlayer ? "player" : "monster");
    BattleActor *returnValue = isPlayer ? &player : &monster;
    returnValue->isPlayer = isPlayer;
    returnValue->combatantClass = combatantClass;
    returnValue->skillList = skillList;
    returnValue->level = level;
    returnValue->speed = CombatantClass_GetSpeed(combatantClass, level);
    returnValue->maxHealth = CombatantClass_GetHealth(combatantClass, level);
    if(startingHealth <= 0)
        returnValue->health = returnValue->maxHealth;
    else
        returnValue->health = startingHealth;
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
