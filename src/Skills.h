#pragma once
#include "CombatantClass.h"
#include "Utils.h"
#include "MiniAdventure.h"
#include "AutoSizeConstants.h"

#define INVALID_SKILL (uint16_t)(-1)

typedef struct CombatantClass CombatantClass;
typedef struct SkillList SkillList;

typedef void (*ResponseCallback)(void); // This needs to take some arguments including character and enemy

// bit field
#define PHYSICAL BIT_FIELD(0)
#define MAGIC BIT_FIELD(1)
#define FIRE BIT_FIELD(2)
#define ICE BIT_FIELD(3)
#define LIGHTNING BIT_FIELD(4)
#define SLASHING BIT_FIELD(5)
#define PIERCING BIT_FIELD(6)
#define BLUDGEONING BIT_FIELD(7)

typedef struct Skill
{
    char name[MAX_STORY_NAME_LENGTH];
    char description[MAX_STORY_DESC_LENGTH];
    uint16_t type;
    uint16_t speed;
    uint16_t damageType;
    uint16_t potency;
    uint16_t cooldown;
} Skill;

typedef struct SkillListEntry
{
    uint16_t id;
    uint16_t level;
    uint16_t cooldown;
} SkillListEntry;

typedef struct SkillList
{
    uint16_t count;
    SkillListEntry entries[MAX_SKILLS_IN_LIST];
} SkillList;

typedef struct NewBattleActor
{
    char name[MAX_STORY_NAME_LENGTH];
    bool isPlayer;
    uint16_t level;
    uint16_t speed;
    uint16_t currentHealth;
    uint16_t maxHealth;
    uint16_t currentTime;
    CombatantClass combatantClass;
    SkillList skillList;
    bool skillQueued;
    uint16_t activeSkill;
    uint16_t counterSkill;
} NewBattleActor;

const char *ExecuteSkill(Skill *skill, NewBattleActor *attacker, NewBattleActor *defender);
void UpdateSkillCooldowns(SkillList *skillList);
