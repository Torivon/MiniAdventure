#include <pebble.h>
#include "BattleActor.h"
#include "CombatantClass.h"
#include "Logging.h"
#include "MiniAdventure.h"
#include "Skills.h"

typedef struct Skill
{
    char *name;
    char *description;
    SkillType type;
    uint16_t speed;
    uint16_t damageType;
    uint16_t potency;
    int cooldown;
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

char *GetSkillDescription(Skill *skill)
{
    if(!skill)
        return NULL;
    
    return skill->description;
}

static Skill fastAttack =
{
    .name = "Fast Attack",
    .description = "Quick stab",
    .type = SKILL_TYPE_BASIC_ATTACK,
    .speed = 100,
    .damageType = PHYSICAL | PIERCING,
    .potency = 1
};

static Skill slowAttack =
{
    .name = "Slow Attack",
    .description = "Heavy slash",
    .type = SKILL_TYPE_BASIC_ATTACK,
    .speed = 20,
    .damageType = PHYSICAL | SLASHING,
    .potency = 5,
    .cooldown = 4
};

static Skill shieldBash =
{
    .name = "Shield Bash",
    .description = "Bash enemy as they attack",
    .type = SKILL_TYPE_COUNTER,
    .speed = 100,
    .damageType = PHYSICAL | BLUDGEONING,
    .potency = 10,
    .cooldown = 2,
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
        case SKILLID_SHIELD_BASH:
        {
            return &shieldBash;
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

    // TODO: Should also deal with damage types and resistances
    return potency;
}

const char *ExecuteSkill(SkillInstance *instance)
{
    static char description[30];
    DEBUG_VERBOSE_LOG("ExecuteSkill");
    Skill *skill = GetSkillByID(instance->entry->id);
    switch(skill->type)
    {
        case SKILL_TYPE_BASIC_ATTACK:
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
