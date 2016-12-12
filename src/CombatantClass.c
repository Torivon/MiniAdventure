#include <pebble.h>

#include "CombatantClass.h"

int CombatantClass_GetStrength(CombatantClass *combatant, int level)
{
    return (combatant->strengthRank + 3) * level;
}

int CombatantClass_GetMagic(CombatantClass *combatant, int level)
{
    return (combatant->magicRank + 3) * level;
}

int CombatantClass_GetDefense(CombatantClass *combatant, int level)
{
    return (combatant->defenseRank + 2) * 10;
}

int CombatantClass_GetMagicDefense(CombatantClass *combatant, int level)
{
    return (combatant->magicDefenseRank + 2) * 10;
}

int CombatantClass_GetSpeed(CombatantClass *combatant, int level)
{
    return (combatant->speedRank + 1) * 5;
}

int CombatantClass_GetHealth(CombatantClass *combatant, int level)
{
    return 15 + 2 * (combatant->healthRank + 3) * level;
}

