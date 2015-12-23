#include "pebble.h"

#include "Skills.h"
#include "CharacterClass.h"
#include "CombatantClass.h"

typedef struct CharacterClass
{
    CombatantClass combatantClass;
    SkillList skillList;
} CharacterClass;

CharacterClass paladinClass =
{
    .combatantClass = {.strengthRank = RANK_B, .magicRank = RANK_C, .defenseRank = RANK_A, .magicDefenseRank = RANK_B, .speedRank = RANK_C, .healthRank = RANK_A},
    .skillList = {.entries = {{.id = SKILLID_FAST_ATTACK, .level = 1}, {.id = SKILLID_SLOW_ATTACK, .level = 1}, {.id = SKILLID_SHIELD_BASH, .level = 2}}, .count = 3},
};

CharacterClass *GetPaladinClass(void)
{
    return &paladinClass;
}

CombatantClass *GetCombatantClass(CharacterClass *class)
{
    if(!class)
        return NULL;
    
    return &class->combatantClass;
}

SkillList *GetSkillList(CharacterClass *class)
{
    if(!class)
        return NULL;
    
    return &class->skillList;
}

