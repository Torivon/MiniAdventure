#pragma once

#include "Skills.h"

typedef struct CharacterClass CharacterClass;
typedef struct CombatantClass CombatantClass;

CharacterClass *GetPaladinClass(void);
CombatantClass *GetCombatantClass(CharacterClass *class);
SkillList *GetSkillList(CharacterClass *class);

