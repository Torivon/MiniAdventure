#pragma once
#include "Utils.h"
#include "MiniAdventure.h"
#include "AutoSizeConstants.h"

typedef struct BattleActor BattleActor;

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

typedef enum
{
    SKILL_TYPE_BASIC_ATTACK,
    SKILL_TYPE_BASIC_HEAL,
    SKILL_TYPE_COUNTER,
    SKILL_TYPE_BUFF,
    SKILL_TYPE_DEBUFF,
} SkillType;

typedef struct Skill
{
    char name[MAX_STORY_NAME_LENGTH];
    char description[MAX_STORY_NAME_LENGTH];
    SkillType type;
    uint16_t speed;
    uint16_t damageType;
    uint16_t potency;
    int cooldown;
} Skill;

typedef struct SkillInstance SkillInstance;

typedef enum
{
    SKILLID_FAST_ATTACK,
    SKILLID_SLOW_ATTACK,
    SKILLID_SHIELD_BASH,
} SkillID;

typedef struct SkillListEntry
{
    SkillID id;
    int level;
    int cooldown;
} SkillListEntry;

typedef struct SkillList
{
    SkillListEntry entries[MAX_SKILLS_IN_LIST];
    int count;
} SkillList;

Skill *GetSkillByID(SkillID id);

char *GetSkillName(Skill *skill);
char *GetSkillDescription(Skill *skill);
uint16_t GetSkillSpeed(Skill *skill);

Skill *GetSkillFromInstance(SkillInstance *instance);

SkillInstance *CreateSkillInstance(SkillListEntry *entry, BattleActor *attacker, BattleActor *defender);
const char *ExecuteSkill(SkillInstance *instance);
void FreeSkillInstance(SkillInstance *instance);
BattleActor *SkillInstanceGetAttacker(SkillInstance *instance);
void UpdateSkillCooldowns(SkillList *skillList);
