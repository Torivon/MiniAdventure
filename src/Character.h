#pragma once

#define XP_TO_LEVEL_UP 1000

typedef struct Character Character;
typedef struct CharacterClass CharacterClass;
typedef struct CombatantClass CombatantClass;
typedef struct SkillList SkillList;

void Character_SetClass(int type);
void Character_SetHealth(int health);
int Character_GetHealth(void);
int Character_GetLevel(void);
void Character_GrantXP(uint16_t monsterLevel);
Character *Character_GetData(void);
void Character_SetCooldowns(uint16_t *cooldowns);
uint16_t *Character_GetCooldowns(void);

void Character_WritePersistedData(int index);
void Character_ReadPersistedData(int index);
size_t Character_GetDataSize(void);


void Character_Initialize(void);
