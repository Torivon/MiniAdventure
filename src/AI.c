#include "pebble.h"

#include "AI.h"
#include "AutoAIConstants.h"
#include "AutoSkillConstants.h"
#include "Battler.h"
#include "Story.h"
#include "Skills.h"

// Skill integers here should be indexes into the enemy's skill list

void ExecuteAI(BattleActorWrapper *attacker, BattleActorWrapper *defender)
{
    AIState *state = &attacker->actor.aiState;
    uint16_t aiType = BattlerWrapper_GetAIType(attacker->battlerWrapper);
    int i = state->skillIndex;
    SkillList *skillList = BattlerWrapper_GetSkillList(attacker->battlerWrapper);
    if(aiType == AI_STAGE_TYPE_RANDOM)
    {
        i = Random(skillList->count);
    }
    do
    {
        SkillListEntry *entry = &skillList->entries[i];
        if(attacker->actor.level >= entry->level && attacker->actor.skillCooldowns[i] == 0)
        {
            //QueueSkill
            attacker->actor.activeSkill = i;
            attacker->actor.skillQueued = true;
            Skill *skill = BattlerWrapper_GetSkillByIndex(attacker->battlerWrapper, attacker->actor.activeSkill);
            attacker->actor.skillCooldowns[attacker->actor.activeSkill] = GetSkillCooldown(skill);

            state->skillIndex = i + 1;
            if(state->skillIndex >= skillList->count)
                state->skillIndex = 0;
            return;
        }
        else
        {
            ++i;
            if(i >= skillList->count)
            {
                i = 0;
            }
        }
    } while(i != state->skillIndex);
}