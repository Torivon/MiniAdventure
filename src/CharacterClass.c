#include "pebble.h"

#if 0

#include "Skills.h"
#include "CharacterClass.h"
#include "CombatantClass.h"

typedef struct CharacterClass
{
    const char *name;
    const char *description;
    CombatantClass combatantClass;
    SkillList skillList;
} CharacterClass;

CharacterClass paladinClass =
{
    .name = "Paladin",
    .description = "Defends the weak with sword and shield. Has access to limited holy magic.",
    .combatantClass = {.strengthRank = RANK_B, .magicRank = RANK_C, .defenseRank = RANK_A, .magicDefenseRank = RANK_B, .speedRank = RANK_C, .healthRank = RANK_A},
    .skillList =
    {
        .entries =
        {
            {.id = SKILLID_FAST_ATTACK, .level = 1},
            {.id = SKILLID_SLOW_ATTACK, .level = 1},
            {.id = SKILLID_SHIELD_BASH, .level = 2}
        },
        .count = 3
    },
};

CharacterClass *classes[] =
{
    &paladinClass,
};

CharacterClass *CharacterClass_GetClassByType(CharacterClassType type)
{
    return classes[type];
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

#endif
