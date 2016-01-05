#include "pebble.h"

#include "Adventure.h"
#include "Character.h"
#include "CharacterClass.h"
#include "CombatantClass.h"
#include "Logging.h"
#include "Utils.h"


typedef struct Character
{
    CharacterClassType classType;
    SkillList skillList;
    int level;
    int currentHealth;
} Character;

Character character;

CharacterClass *Character_GetClass(void)
{
    return CharacterClass_GetClassByType(character.classType);
}

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

void Character_UpdateSkillList(void)
{
    SkillList *classSkillList = GetSkillList(Character_GetClass());
    if(character.skillList.count < classSkillList->count)
    {
        int index = character.skillList.count;
        while(character.skillList.count < classSkillList->count && classSkillList->entries[index].level <= character.level)
        {
            character.skillList.entries[index] = classSkillList->entries[index];
            character.skillList.count++;
            index = character.skillList.count;
        }
    }
}

SkillList *Character_GetSkillList(void)
{
    Character_UpdateSkillList();
    return character.skillList;
}

CombatantClass *Character_GetCombatantClass(void)
{
    return GetCombatantClass(Character_GetClass());
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
}

void Character_WritePersistedData(int index)
{
	persist_write_data(index, &character, sizeof(Character));
}

void Character_Initialize(void)
{
    INFO_LOG("Initializing character.");
    character.classType = CLASS_PALADIN;
    character.level = 2;
    Character_UpdateSkillList();
    character.currentHealth = CombatantClass_GetHealth(GetCombatantClass(Character_GetClass()), character.level);
}
