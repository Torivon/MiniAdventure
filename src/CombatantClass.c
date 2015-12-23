#include "pebble.h"

#include "CombatantClass.h"

static int PlaceholderData(CombatantRank rank, int level)
{
  return rank * level + 1;
}

int CombatantClass_GetStrength(CombatantClass *combatant, int level)
{
  return PlaceholderData(combatant->strengthRank, level);
}

int CombatantClass_GetMagic(CombatantClass *combatant, int level)
{
  return PlaceholderData(combatant->magicRank, level);
}

int CombatantClass_GetDefense(CombatantClass *combatant, int level)
{
  return PlaceholderData(combatant->defenseRank, level);
}

int CombatantClass_GetMagicDefense(CombatantClass *combatant, int level)
{
  return PlaceholderData(combatant->magicDefenseRank, level);
}

int CombatantClass_GetSpeed(CombatantClass *combatant, int level)
{
  return combatant->speedRank * 5;
}

int CombatantClass_GetHealth(CombatantClass *combatant, int level)
{
  return 10 * PlaceholderData(combatant->healthRank, level);
}

