#include <pebble.h>
#include "AutoSkillConstants.h"
#include "Battler.h"
#include "BinaryResourceLoading.h"
#include "CombatantClass.h"
#include "Logging.h"
#include "MiniAdventure.h"
#include "Skills.h"
#include "Story.h"

void Skill_Free(Skill *skill)
{
    if(skill)
        free(skill);
}

Skill *Skill_Load(uint16_t logical_index)
{
    ResHandle currentStoryData = Story_GetCurrentResHandle();
    Skill *newSkill = calloc(sizeof(Skill), 1);
    ResourceLoadStruct(currentStoryData, logical_index, (uint8_t*)newSkill, sizeof(Skill), "Skill");
    return newSkill;
}

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

static int ComputeSkillPotency(Skill *skill, BattleActorWrapper *attacker, BattleActorWrapper *defender)
{
    int attackPower = 0;
    int defensePower = 0;
    
    // Compute immunity first. If the defender is immune, return 0
    if(defender && BattlerWrapper_CheckImmunity(defender->battlerWrapper, skill->damageType))
        return 0;
    
    CombatantClass *attackerCombatantClass = BattlerWrapper_GetCombatantClass(attacker->battlerWrapper);
    CombatantClass *defenderCombatantClass = defender ? BattlerWrapper_GetCombatantClass(defender->battlerWrapper) : NULL;
    
    if(skill->damageType & PHYSICAL)
    {
        attackPower = CombatantClass_GetStrength(attackerCombatantClass, attacker->actor.level);
        if(defender)
            defensePower = CombatantClass_GetDefense(defenderCombatantClass, defender->actor.level);
    }
    else if(skill->damageType & MAGIC)
    {
        attackPower = CombatantClass_GetMagic(attackerCombatantClass, attacker->actor.level);
        if(defender)
            defensePower = CombatantClass_GetMagicDefense(defenderCombatantClass, defender->actor.level);
    }
    
    int baseDamage = (skill->potency + (attackPower * skill->potency)) / 10;
    
    int damage = (baseDamage * (100 - defensePower)) / 100;
    
    if(damage <= 0 && skill->potency > 0)
        damage = 1;
    
    if(defender && BattlerWrapper_CheckVulnerability(defender->battlerWrapper, skill->damageType))
        damage *= 2;

    if(defender && BattlerWrapper_CheckResistance(defender->battlerWrapper, skill->damageType))
        damage /= 2;
    
    if(defender && BattlerWrapper_CheckAbsorption(defender->battlerWrapper, skill->damageType))
        damage = -damage;
    
    return damage;
}

void DealDamage(int potency, BattleActor *defender)
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

void ApplyStatus(Skill *skill, BattleActorWrapper *target)
{
    if(skill->skillProperties > 0)
    {
        for(int i = 0; i < MAX_STATUS_EFFECTS; ++i)
        {
            uint16_t bit = 1 << i;
            if(skill->skillProperties & bit && (!BattlerWrapper_CheckStatusImmunity(target->battlerWrapper, bit)))
            {
                target->actor.statusEffectDurations[i] = skill->propertyDuration;
                if(i == STATUS_EFFECT_STUN)
                {
                    target->actor.skillQueued = false;
                    target->actor.currentTime = 0;
                }
            }
        }
    }
}

const char *ExecuteSkill(Skill *skill, BattleActorWrapper *attacker, BattleActorWrapper *defender)
{
    static char description[MAX_DIALOG_LENGTH];
    DEBUG_VERBOSE_LOG("ExecuteSkill");
    
    switch(skill->target)
    {
        case SKILL_TARGET_ENEMY:
        {
            bool countered = false;
            if(defender->actor.counterSkill != INVALID_SKILL)
            {
                Skill *counterSkill = BattlerWrapper_GetSkillByIndex(defender->battlerWrapper, defender->actor.counterSkill);
                if(counterSkill->counterDamageType & skill->damageType)
                {
                    int potency = ComputeSkillPotency(counterSkill, defender, attacker);
                    DealDamage(potency, &attacker->actor);
                    snprintf(description, sizeof(description), "%s counters for %d damage with %s", BattlerWrapper_GetBattlerName(defender->battlerWrapper), potency, counterSkill->description);
                    countered = true;
                    ApplyStatus(counterSkill, attacker);
                }
                defender->actor.counterSkill = INVALID_SKILL;
            }
            
            if(!countered)
            {
                int potency = ComputeSkillPotency(skill, attacker, defender);
                DealDamage(potency, &defender->actor);
                snprintf(description, sizeof(description), "%s deals %d damage with %s", BattlerWrapper_GetBattlerName(attacker->battlerWrapper), potency, skill->description);
                ApplyStatus(skill, defender);
            }
            break;
        }
        case SKILL_TARGET_COUNTER:
        {
            attacker->actor.counterSkill = attacker->actor.activeSkill;
            snprintf(description, sizeof(description), "%s prepares %s", BattlerWrapper_GetBattlerName(attacker->battlerWrapper), skill->name);
            break;
        }
        case SKILL_TARGET_SELF:
        {
            int potency = ComputeSkillPotency(skill, attacker, NULL);
            DealDamage(-potency, &attacker->actor);
            ApplyStatus(skill, attacker);
            snprintf(description, sizeof(description), "%s heals %d damage with %s", BattlerWrapper_GetBattlerName(attacker->battlerWrapper), potency, skill->description);
            break;
        }
        default:
        {
            snprintf(description, sizeof(description), "Unhandled skill: %s", skill->name);
            break;
        }
    }
    DEBUG_VERBOSE_LOG("Setting description: %s", description);
    attacker->actor.activeSkill = INVALID_SKILL;
    return description;
}

void UpdateSkillCooldowns(uint16_t *skillCooldowns)
{
    for(int i = 0; i < MAX_SKILLS_IN_LIST; ++i)
    {
        if(skillCooldowns[i] > 0)
            skillCooldowns[i]--;
    }
}
