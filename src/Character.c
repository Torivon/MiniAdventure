#include "pebble.h"

#include "Adventure.h"
#include "Character.h"
#include "CharacterClass.h"
#include "CombatantClass.h"
#include "Logging.h"
#include "ResourceStory.h"
#include "Utils.h"

typedef struct Character
{
    uint16_t classType;
    int level;
    int currentHealth;
} Character;

Character character;

void Character_SetClass(int type)
{
    character.classType = type;
}

int Character_GetHealth(void)
{
    return character.currentHealth;
}

void Character_SetHealth(int health)
{
    character.currentHealth = health;
}

SkillList *Character_GetSkillList(void)
{
    return ResourceStory_GetCurrentPlayerSkillList();
}

CombatantClass *Character_GetCombatantClass(void)
{
    return ResourceStory_GetCurrentPlayerCombatantClass();
}

int Character_GetLevel(void)
{
    return character.level;
}

Character *GetCharacter(void)
{
    return &character;
}

size_t Character_GetDataSize(void)
{
	return sizeof(Character);
}

void Character_ReadPersistedData(int index)
{
	persist_read_data(index, &character, sizeof(Character));	
    ResourceBattler_LoadPlayer(character.classType);
}

void Character_WritePersistedData(int index)
{
	persist_write_data(index, &character, sizeof(Character));
}

void Character_Initialize(void)
{
    INFO_LOG("Initializing character.");
    character.classType = 0; // Get the first class in the story
    ResourceBattler_LoadPlayer(character.classType);
    character.level = 2;
    character.currentHealth = CombatantClass_GetHealth(ResourceStory_GetCurrentPlayerCombatantClass(), character.level);
}
