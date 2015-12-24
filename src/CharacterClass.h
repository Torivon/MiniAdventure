#pragma once

#include "Skills.h"

typedef struct CharacterClass CharacterClass;
typedef struct CombatantClass CombatantClass;

typedef enum
{
    CLASS_PALADIN,
} CharacterClassType;

CharacterClass *CharacterClass_GetClassByType(CharacterClassType type);

CharacterClass *GetPaladinClass(void);
CombatantClass *GetCombatantClass(CharacterClass *class);
SkillList *GetSkillList(CharacterClass *class);

