#include <pebble.h>
#include "BattleActor.h"
#include "Logging.h"
#include "MiniAdventure.h"
#include "Skills.h"

typedef struct Skill
{
	char *name;
	uint16_t speed;
	uint16_t damageType;
	uint16_t potency;
} Skill;

typedef struct SkillInstance
{
	Skill *skill;
	BattleActor *attacker;
	BattleActor *defender;
	bool active;
} SkillInstance;

static SkillInstance instances[MAX_BATTLE_QUEUE];

char *GetSkillName(Skill *skill)
{
	if(!skill)
		return NULL;
	
	return skill->name;
}

static Skill fastAttack = 
{
	.name = "Fast Attack", 
	.speed = 100, 
	.damageType = PHYSICAL | PIERCING, 
	.potency = 1
};

Skill *GetFastAttack(void)
{
	return &fastAttack;
}

static Skill slowAttack = 
{
	.name = "Slow Attack", 
	.speed = 20, 
	.damageType = PHYSICAL | SLASHING, 
	.potency = 10
};

Skill *GetSlowAttack(void)
{
	return &slowAttack;
}

uint16_t GetSkillSpeed(Skill *skill)
{
	return skill->speed;
}

SkillInstance *CreateSkillInstance(Skill *skill, BattleActor *attacker, BattleActor *defender)
{
	for(int i = 0; i < MAX_BATTLE_QUEUE; ++i)
	{
		if(!instances[i].active)
		{
			SkillInstance *newInstance = &instances[i];
			newInstance->active = true;
			newInstance->skill = skill;
			newInstance->attacker = attacker;
			newInstance->defender = defender;
			return newInstance;			
		}
	}
	
	return NULL;
}

Skill *GetSkillFromInstance(SkillInstance *instance)
{
	return instance->skill;
}

const char *ExecuteSkill(SkillInstance *instance)
{
	static char description[30];
	DEBUG_VERBOSE_LOG("ExecuteSkill");
	DealDamage(instance->skill->potency, instance->defender);
	snprintf(description, sizeof(description), "%s takes %d damage", BattleActor_IsPlayer(instance->defender) ? "Player" : "Monster", instance->skill->potency);
	DEBUG_VERBOSE_LOG("Setting description: %s", description);
	FreeSkillInstance(instance);
	return description;
}

void FreeSkillInstance(SkillInstance *instance)
{
	instance->active = false;
}

BattleActor *SkillInstanceGetAttacker(SkillInstance *instance)
{
	return instance->attacker;
}
