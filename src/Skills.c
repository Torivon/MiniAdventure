#include <pebble.h>
#include "BattleActor.h"
#include "Skills.h"

typedef struct Skill
{
	char *name;
	uint16_t speed;
	uint16_t damageType;
	uint16_t potency;
} Skill;

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

void ExecuteSkill(Skill *skill, BattleActor *attacker, BattleActor *defender)
{
	DealDamage(skill->potency, defender);
}
