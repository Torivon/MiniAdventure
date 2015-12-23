#pragma once

typedef enum
{
  RANK_F = 0,
  RANK_E,
  RANK_D,
  RANK_C,
  RANK_B,
  RANK_A,
  RANK_S,
} CombatantRank;

typedef struct CombatantClass 
{
  CombatantRank strengthRank;
  CombatantRank magicRank;
  CombatantRank defenseRank;
  CombatantRank magicDefenseRank;
  CombatantRank speedRank;
  CombatantRank healthRank;
} CombatantClass;

int CombatantClass_GetStrength(CombatantClass *combatant, int level);
int CombatantClass_GetMagic(CombatantClass *combatant, int level);
int CombatantClass_GetDefense(CombatantClass *combatant, int level);
int CombatantClass_GetMagicDefense(CombatantClass *combatant, int level);
int CombatantClass_GetSpeed(CombatantClass *combatant, int level);
int CombatantClass_GetHealth(CombatantClass *combatant, int level);

