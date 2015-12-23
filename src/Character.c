#include "pebble.h"

#include "Adventure.h"
#include "Character.h"
#include "CharacterClass.h"
#include "Logging.h"
#include "Utils.h"

CharacterData characterData;

typedef struct Character
{
  CharacterClass *class;
  SkillList skillList;
  int level;
} Character;

Character character;

void UpdateSkillList(Character *character)
{
  SkillList *classSkillList = GetSkillList(character->class);
  if(character->skillList.count < classSkillList->count)
    {
      int index = character->skillList.count;
      while(classSkillList->entries[index].level <= character->level && character->skillList.count <= classSkillList->count)
	{
	  character->skillList.entries[index] = classSkillList->entries[index];
	  character->skillList.count++;
	  index = character->skillList.count;
	}
    }
}

SkillList *CharacterGetSkillList(Character *character)
{
  UpdateSkillList(character);
  return &character->skillList;
}

CombatantClass *CharacterGetCombatantClass(Character *character)
{
  return GetCombatantClass(character->class);
}

int CharacterGetLevel(Character *character)
{
  return character->level;
}

Character *GetNewCharacter(void)
{
  return &character;
}

void AddStatPointToSpend(void)
{
	++characterData.statPointsToSpend;
}

const char *UpdateLevelText(void)
{
	static char levelText[] = "00"; // Needs to be static because it's used by the system later.
	IntToString(levelText, 2, characterData.level);
	return levelText;
}

const char *UpdateXPText(void)
{
	static char xpText[] = "0000"; // Needs to be static because it's used by the system later.
	IntToString(xpText, 4, characterData.xp);
	return xpText;
}

const char *UpdateNextXPText(void)
{
	static char nextXPText[] = "0000"; // Needs to be static because it's used by the system later.
	IntToString(nextXPText, 4, characterData.xpForNextLevel);
	return nextXPText;
}

const char *UpdateGoldText(void)
{
	static char goldText[] = "00000"; // Needs to be static because it's used by the system later.
	IntToString(goldText, 5, characterData.gold);
	return goldText;
}

const char *UpdateEscapeText(void)
{
	static char escapeText[] = "000"; // Needs to be static because it's used by the system later.
	IntToString(escapeText, 3, characterData.escapes);
	return escapeText;
}

void IncrementEscapes(void)
{
	++characterData.escapes;
}

int ComputePlayerHealth(int level)
{
	return 10 + ((level-1)*(level)/2) + ((level-1)*(level)*(level+1)/(6*32));
}

int ComputeXPForNextLevel(int level)
{
	return XP_FOR_NEXT_LEVEL;
}

void InitializeCharacter(void)
{
	INFO_LOG("Initializing character.");
	characterData.xp = 0;
	characterData.level = 1;
	characterData.gold = 0;
	characterData.escapes = 0;
	characterData.xpForNextLevel = ComputeXPForNextLevel(1);
	characterData.stats.maxHealth = ComputePlayerHealth(1);
	characterData.stats.currentHealth = characterData.stats.maxHealth;
	characterData.stats.strength = 1;
	characterData.stats.magic = 1;
	characterData.stats.defense = 1;
	characterData.stats.magicDefense = 1;
	characterData.statPointsToSpend = 0;
	characterData.speed = 10;

	character.class = GetPaladinClass();
	character.level = 1;
}

// Returns true on levelup
bool GrantExperience(int xp)
{
	characterData.xp += xp;
	if(characterData.xp >= characterData.xpForNextLevel)
	{
		return true;
	}
	return false;
}

void GrantGold(int gold)
{
	characterData.gold += gold;
	if(characterData.gold > 9999)
		characterData.gold = 9999;
}

void DealPlayerDamage(int damage)
{
	characterData.stats.currentHealth = characterData.stats.currentHealth - damage;
	if(characterData.stats.currentHealth <= 0)
	{
		characterData.stats.currentHealth = 0;
	}
}

CharacterData *GetCharacter(void)
{
	return &characterData;
}

void HealPlayerByPercent(int percent)
{
	characterData.stats.currentHealth += (characterData.stats.maxHealth * percent) / 100;
	if(characterData.stats.currentHealth > characterData.stats.maxHealth)
		characterData.stats.currentHealth = characterData.stats.maxHealth;
}

bool PlayerIsInjured(void)
{
	return characterData.stats.currentHealth < characterData.stats.maxHealth;
}

bool PlayerIsDead(void)
{
	return characterData.stats.currentHealth <= 0;
}

const char  *UpdateStatPointText(void)
{
	static char statText[] = "00"; // Needs to be static because it's used by the system later.

	IntToString(statText, 2, characterData.statPointsToSpend);
	return statText;
}

const char  *UpdateStrengthText(void)
{
	static char strengthText[] = "00"; // Needs to be static because it's used by the system later.

	IntToString(strengthText, 2, characterData.stats.strength);
	return strengthText;
}

const char  *UpdateDefenseText(void)
{
	static char defenseText[] = "00"; // Needs to be static because it's used by the system later.

	IntToString(defenseText, 2, characterData.stats.defense);
	return defenseText;
}

const char  *UpdateMagicText(void)
{
	static char magicText[] = "00"; // Needs to be static because it's used by the system later.

	IntToString(magicText, 2, characterData.stats.magic);
	return magicText;
}

const char  *UpdateMagicDefenseText(void)
{
	static char magicDefenseText[] = "00"; // Needs to be static because it's used by the system later.

	IntToString(magicDefenseText, 2, characterData.stats.magicDefense);
	return magicDefenseText;
}

void IncrementStat(int *stat)
{
	if(characterData.statPointsToSpend && (*stat) < characterData.level)
	{
		++(*stat);
		--characterData.statPointsToSpend;
	}
}

void IncrementStrength(void)
{
	IncrementStat(&characterData.stats.strength);
}

void IncrementDefense(void)
{
	IncrementStat(&characterData.stats.defense);
}

void IncrementMagic(void)
{
	IncrementStat(&characterData.stats.magic);
}

void IncrementMagicDefense(void)
{
	IncrementStat(&characterData.stats.magicDefense);
}

void LevelUpData(void)
{
	characterData.statPointsToSpend += STAT_POINTS_PER_LEVEL;
	++characterData.level;
	characterData.xpForNextLevel += ComputeXPForNextLevel(characterData.level);
	characterData.stats.maxHealth = ComputePlayerHealth(characterData.level);
	if(characterData.stats.maxHealth > 9999)
		characterData.stats.maxHealth = 9999;
	characterData.stats.currentHealth = characterData.stats.maxHealth;
}

void LevelUp(void)
{
	INFO_LOG("Level up.");
	LevelUpData();
}

void ForceLevelUp(void)
{
	GrantExperience(characterData.xpForNextLevel - characterData.xp);
	LevelUp();
}

void ForceGold(void)
{
	GrantGold(100);
}
