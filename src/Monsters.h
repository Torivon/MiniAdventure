#pragma once

#include "Character.h"
#include "CombatantClass.h"
#include "Skills.h"

typedef struct MonsterDef
{
    const char *name;
    int imageId;
    int extraFireDefenseMultiplier : 11;
    int extraIceDefenseMultiplier : 11;
    int extraLightningDefenseMultiplier : 11;
    bool allowMagicAttack : 1;
    bool allowPhysicalAttack : 1;
    int powerLevel : 3;
    int healthLevel : 3;
    int defenseLevel : 3;
    int magicDefenseLevel : 3;
    int goldScale : 5;
    bool preventRun : 1;
    int speed;
    CombatantClass combatantClass;
    SkillList skillList;
} MonsterDef;

int GetMostRecentMonster(void);

int GetMonsterDefense(int defenseLevel);
int ScaleMonsterHealth(MonsterDef *monster, int baseHealth);
int GetMonsterPowerDivisor(int powerLevel);
CombatantClass *Monster_GetCombatantClass(MonsterDef *monster);
SkillList *Monster_GetSkillList(MonsterDef *monster);
