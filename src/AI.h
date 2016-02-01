#pragma once

#include "AutoSizeConstants.h"

typedef struct BattleActorWrapper BattleActorWrapper;

typedef struct AIStage
{
    uint16_t stage_type;
    uint16_t skills[MAX_SKILLS_PER_AI_STAGE];
} AIStage;

typedef struct AIInterrupt
{
    uint16_t target;
    uint16_t trait;
    uint16_t value;
    uint16_t skill;
} AIInterrupt;

typedef struct AI
{
    AIInterrupt interrupts[MAX_AI_INTERRUPTS];
    AIStage stages[MAX_AI_STAGES];
} AI;

void ExecuteAI(BattleActorWrapper *attacker, BattleActorWrapper *defender);
