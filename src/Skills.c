#include <pebble.h>
#include "AutoSkillConstants.h"
#include "CombatantClass.h"
#include "Logging.h"
#include "MiniAdventure.h"
#include "ResourceStory.h"
#include "Skills.h"

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

static int ComputeSkillPotency(Skill *skill, NewBattleActor *attacker, NewBattleActor *defender)
{
    int attackPower = 1;
    int defensePower = 1;
    if(skill->damageType & PHYSICAL)
    {
        attackPower = CombatantClass_GetStrength(&attacker->combatantClass, attacker->level);
        defensePower = CombatantClass_GetDefense(&defender->combatantClass, defender->level);
    }
    else if(skill->damageType & MAGIC)
    {
        attackPower = CombatantClass_GetMagic(&attacker->combatantClass, attacker->level);
        defensePower = CombatantClass_GetMagicDefense(&defender->combatantClass, defender->level);
    }
    
    if(defensePower == 0)
        defensePower = 1;
    
    int potency = skill->potency * attackPower / defensePower;
    
    if(potency <= 0)
        potency = 1;

    // TODO: Should also deal with damage types and resistances
    return potency;
}

static void DealDamage(int potency, NewBattleActor *defender)
{
    if(!defender)
        return;
    if(potency > 0 && defender->currentHealth < potency)
        defender->currentHealth = 0;
    else
        defender->currentHealth -= potency;
    if(defender->currentHealth > defender->maxHealth)
        defender->currentHealth = defender->maxHealth;
}

const char *ExecuteSkill(Skill *skill, NewBattleActor *attacker, NewBattleActor *defender)
{
    static char description[30];
    DEBUG_VERBOSE_LOG("ExecuteSkill");
    
    switch(skill->type)
    {
        case SKILL_TYPE_ATTACK:
        {
            if(defender->counterSkill != INVALID_SKILL)
            {
                //TODO: Add an attack type to counters, so that we can differentiate between counters to magic attacks and counters to physical attacks
                Skill *counterSkill = ResourceStory_GetLoadedSkillByID(defender->isPlayer, defender->counterSkill);
                int potency = ComputeSkillPotency(counterSkill, defender, attacker);
                DealDamage(potency, attacker);
                snprintf(description, sizeof(description), "%s counters for %d damage", defender->name, potency);

                defender->counterSkill = INVALID_SKILL;
            }
            else
            {
                int potency = ComputeSkillPotency(skill, attacker, defender);
                DealDamage(potency, defender);
                snprintf(description, sizeof(description), "%s takes %d damage", defender->name, potency);
            }
            break;
        }
        case SKILL_TYPE_COUNTER:
        {
            attacker->counterSkill = attacker->activeSkill;
            snprintf(description, sizeof(description), "%s prepares %s", attacker->name, skill->name);
            break;
        }
        case SKILL_TYPE_HEAL:
        {
            int potency = ComputeSkillPotency(skill, attacker, defender);
            DealDamage(-potency, attacker);
            snprintf(description, sizeof(description), "%s heals %d damage", attacker->name, potency);
            break;
        }
        default:
        {
            snprintf(description, sizeof(description), "Unhandled skill: %s", skill->name);
            break;
        }
    }
    DEBUG_VERBOSE_LOG("Setting description: %s", description);
    attacker->skillList.entries[attacker->activeSkill].cooldown = GetSkillCooldown(skill);
    attacker->activeSkill = INVALID_SKILL;
    return description;
}

void UpdateSkillCooldowns(SkillList *skillList)
{
    for(int i = 0; i < skillList->count; ++i)
    {
        if(skillList->entries[i].cooldown > 0)
            skillList->entries[i].cooldown--;
    }
}
