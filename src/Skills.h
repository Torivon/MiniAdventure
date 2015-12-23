#pragma once
#include "Utils.h"
#include "MiniAdventure.h"

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

typedef struct Skill Skill;
	
typedef struct SkillInstance SkillInstance;

typedef enum
{
  SKILLID_FAST_ATTACK,
  SKILLID_SLOW_ATTACK,
  SKILLID_COUNTER,
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
uint16_t GetSkillSpeed(Skill *skill);
Skill *GetFastAttack(void);
Skill *GetSlowAttack(void);

Skill *GetSkillFromInstance(SkillInstance *instance);

SkillInstance *CreateSkillInstance(SkillListEntry *entry, BattleActor *attacker, BattleActor *defender);
const char *ExecuteSkill(SkillInstance *instance);
void FreeSkillInstance(SkillInstance *instance);
BattleActor *SkillInstanceGetAttacker(SkillInstance *instance);
void UpdateSkillCooldowns(SkillList *skillList);
