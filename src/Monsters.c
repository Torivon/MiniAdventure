#include "pebble.h"

#include "Logging.h"
#include "Monsters.h"
#include "Utils.h"

CombatantClass *Monster_GetCombatantClass(MonsterDef *monster)
{
    return &monster->combatantClass;
}

SkillList *Monster_GetSkillList(MonsterDef *monster)
{
    return &monster->skillList;
}
