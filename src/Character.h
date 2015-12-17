#pragma once

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
