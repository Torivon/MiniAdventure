#pragma once

typedef struct Character Character;
typedef struct CharacterClass CharacterClass;
typedef struct CombatantClass CombatantClass;
typedef struct SkillList SkillList;

CharacterClass *Character_GetClass(void);
void Character_SetClass(int type);
void Character_SetHealth(int health);
int Character_GetHealth(void);
void Character_UpdateSkillList(void);
SkillList *Character_GetSkillList(void);
CombatantClass *Character_GetCombatantClass(void);
int Character_GetLevel(void);
Character *Character_GetData(void);

void Character_WritePersistedData(int index);
void Character_ReadPersistedData(int index);
size_t Character_GetDataSize(void);


void Character_Initialize(void);
