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
Character *GetNewCharacter(void);


typedef struct
{
    int maxHealth;
    int currentHealth;
    int strength;
    int magic;
    int defense;
    int magicDefense;
} Stats;

typedef struct
{
    int xp;
    int xpForNextLevel;
    int level;
    int gold;
    int escapes;
    Stats stats;
    int statPointsToSpend;
    int speed;
} CharacterData;



int ComputePlayerHealth(int level);
void AddStatPointToSpend(void);
void IncrementEscapes(void);

void InitializeCharacter(void);

void DealPlayerDamage(int damage);
CharacterData *GetCharacter(void);

void HealPlayerByPercent(int percent);
bool PlayerIsInjured(void);
bool PlayerIsDead(void);

bool GrantExperience(int exp);
void GrantGold(int gold);

void LevelUpData(void);
void LevelUp(void);
