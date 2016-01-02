#pragma once
#if 0
#include "Character.h"
#include "CombatantClass.h"
#include "Skills.h"

typedef struct MonsterDef
{
    const char *name;
    int imageId;
    CombatantClass combatantClass;
    SkillList skillList;
} MonsterDef;

CombatantClass *Monster_GetCombatantClass(MonsterDef *monster);
SkillList *Monster_GetSkillList(MonsterDef *monster);
#endif