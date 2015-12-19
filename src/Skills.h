#pragma once
#include "Utils.h"

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

char *GetSkillName(Skill *skill);
uint16_t GetSkillSpeed(Skill *skill);
Skill *GetFastAttack(void);
Skill *GetSlowAttack(void);

Skill *GetSkillFromInstance(SkillInstance *instance);

SkillInstance *CreateSkillInstance(Skill *skill, BattleActor *attacker, BattleActor *defender);
const char *ExecuteSkill(SkillInstance *instance);
void FreeSkillInstance(SkillInstance *instance);
BattleActor *SkillInstanceGetAttacker(SkillInstance *instance);
