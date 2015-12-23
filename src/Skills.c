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
        int cooldown;
  bool counterAttack;
} Skill;

typedef struct SkillInstance
{
	SkillListEntry *entry;
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

static Skill slowAttack = 
{
	.name = "Slow Attack", 
	.speed = 20, 
	.damageType = PHYSICAL | SLASHING, 
	.potency = 10,
	.cooldown = 4
};

static Skill counter = 
{
	.name = "Counter", 
	.speed = 100, 
	.damageType = PHYSICAL | SLASHING, 
	.potency = 10,
	.cooldown = 2,
	.counterAttack = true,
};

Skill *GetSkillByID(SkillID id)
{
  switch(id)
  {
  case SKILLID_FAST_ATTACK:
    {
      return &fastAttack;
    }
  case SKILLID_SLOW_ATTACK:
    {
      return &slowAttack;
    }
  case SKILLID_COUNTER:
    {
      return &counter;
    }
  }

  return NULL;
}

uint16_t GetSkillSpeed(Skill *skill)
{
	return skill->speed;
}

int GetSkillCooldown(Skill *skill)
{
  return skill->cooldown;
}

SkillInstance *CreateSkillInstance(SkillListEntry *entry, BattleActor *attacker, BattleActor *defender)
{
	for(int i = 0; i < MAX_BATTLE_QUEUE; ++i)
	{
		if(!instances[i].active)
		{
			SkillInstance *newInstance = &instances[i];
			newInstance->active = true;
			newInstance->entry = entry;
			newInstance->attacker = attacker;
			newInstance->defender = defender;
			return newInstance;			
		}
	}
	
	return NULL;
}

Skill *GetSkillFromInstance(SkillInstance *instance)
{
	Skill *skill = GetSkillByID(instance->entry->id);
	return skill;
}

const char *ExecuteSkill(SkillInstance *instance)
{
	static char description[30];
	DEBUG_VERBOSE_LOG("ExecuteSkill");
	Skill *skill = GetSkillByID(instance->entry->id);
	DealDamage(skill->potency, instance->defender);
	snprintf(description, sizeof(description), "%s takes %d damage", BattleActor_IsPlayer(instance->defender) ? "Player" : "Monster", skill->potency);
	DEBUG_VERBOSE_LOG("Setting description: %s", description);
	instance->entry->cooldown = GetSkillCooldown(skill);
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

void UpdateSkillCooldowns(SkillList *skillList)
{
  for(int i = 0; i < skillList->count; ++i)
    {
      if(skillList->entries[i].cooldown > 0)
	skillList->entries[i].cooldown--;
    }
}
