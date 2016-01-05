#include <pebble.h>
#include "AutoSkillConstants.h"
#include "BattleActor.h"
#include "CombatantClass.h"
#include "Logging.h"
#include "MiniAdventure.h"
#include "ResourceStory.h"
#include "Skills.h"

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

char *GetSkillDescription(Skill *skill)
{
    if(!skill)
        return NULL;
    
    return skill->description;
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
    Skill *skill = ResourceStory_GetLoadedSkillByID(BattleActor_IsPlayer(instance->attacker), instance->entry->id);
    return skill;
}

static int ComputeSkillPotency(SkillInstance *instance)
{
    Skill *skill = GetSkillFromInstance(instance);
    int attackPower = 1;
    int defensePower = 1;
    if(skill->damageType & PHYSICAL)
    {
        attackPower = CombatantClass_GetStrength(BattleActor_GetCombatantClass(instance->attacker), BattleActor_GetLevel(instance->attacker));
        defensePower = CombatantClass_GetDefense(BattleActor_GetCombatantClass(instance->defender), BattleActor_GetLevel(instance->defender));
    }
    else if(skill->damageType & MAGIC)
    {
        attackPower = CombatantClass_GetMagic(BattleActor_GetCombatantClass(instance->attacker), BattleActor_GetLevel(instance->attacker));
        defensePower = CombatantClass_GetMagicDefense(BattleActor_GetCombatantClass(instance->defender), BattleActor_GetLevel(instance->defender));
    }
    
    if(defensePower == 0)
        defensePower = 1;
    
    int potency = skill->potency * attackPower / defensePower;
    
    if(potency <= 0)
        potency = 1;

    // TODO: Should also deal with damage types and resistances
    return potency;
}

const char *ExecuteSkill(SkillInstance *instance)
{
    static char description[30];
    DEBUG_VERBOSE_LOG("ExecuteSkill");
    Skill *skill = GetSkillFromInstance(instance);
    
    switch(skill->type)
    {
        case SKILL_TYPE_ATTACK:
        {
            SkillInstance *counterInstance = BattleActor_GetCounter(instance->defender);
            if(counterInstance)
            {
                //TODO: Add an attack type to counters, so that we can differentiate between counters to magic attacks and counters to physical attacks
                int potency = ComputeSkillPotency(counterInstance);
                DealDamage(potency, counterInstance->defender);
                snprintf(description, sizeof(description), "%s counters for %d damage", BattleActor_IsPlayer(counterInstance->attacker) ? "Player" : "Monster", potency);

                BattleActor_SetCounter(instance->defender, NULL);
                FreeSkillInstance(instance);
                FreeSkillInstance(counterInstance);
            }
            else
            {
                int potency = ComputeSkillPotency(instance);
                DealDamage(potency, instance->defender);
                snprintf(description, sizeof(description), "%s takes %d damage", BattleActor_IsPlayer(instance->defender) ? "Player" : "Monster", potency);
                FreeSkillInstance(instance);
            }
            break;
        }
        case SKILL_TYPE_COUNTER:
        {
            BattleActor_SetCounter(instance->attacker, instance);
            snprintf(description, sizeof(description), "%s prepares %s", BattleActor_IsPlayer(instance->attacker) ? "Player" : "Monster", GetSkillName(GetSkillFromInstance(instance)));
            break;
        }
        case SKILL_TYPE_HEAL:
        {
            int potency = ComputeSkillPotency(instance);
            DealDamage(-potency, instance->attacker);
            snprintf(description, sizeof(description), "%s heals %d damage", BattleActor_IsPlayer(instance->attacker) ? "Player" : "Monster", potency);
            FreeSkillInstance(instance);
            break;
        }
        default:
        {
            snprintf(description, sizeof(description), "Unhandled skill: %s", GetSkillName(GetSkillFromInstance(instance)));
            FreeSkillInstance(instance);
            break;
        }
    }
    DEBUG_VERBOSE_LOG("Setting description: %s", description);
    instance->entry->cooldown = GetSkillCooldown(skill);
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
