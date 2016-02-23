//
//  Battler.h
//  
//
//  Created by Jonathan Panttaja on 1/26/16.
//
//

#pragma once

typedef struct BattlerWrapper BattlerWrapper;
typedef struct Skill Skill;
typedef struct SkillList SkillList;
typedef struct CombatantClass CombatantClass;

void BattlerWrapper_TriggerEvent(BattlerWrapper *wrapper);
uint16_t BattlerWrapper_GetUsableSkillCount(BattlerWrapper *wrapper, uint16_t level);
Skill *BattlerWrapper_GetSkillByIndex(BattlerWrapper *wrapper, uint16_t index);

CombatantClass *BattlerWrapper_GetCombatantClass(BattlerWrapper *wrapper);
BattlerWrapper *BattlerWrapper_GetPlayerWrapper(void);
BattlerWrapper *BattlerWrapper_GetMonsterWrapper(void);
const char *BattlerWrapper_GetBattlerName(BattlerWrapper *wrapper);
SkillList *BattlerWrapper_GetSkillList(BattlerWrapper *wrapper);

void Battler_LoadPlayer(uint16_t classId);
void Battler_UnloadPlayer(void);

bool Monster_LoadCurrent(uint16_t logical_index);
bool Monster_Loaded(void);
void Monster_UnloadCurrent(void);
const char *Monster_GetCurrentName(void);
int16_t Monster_GetCurrentImage(void);

bool BattlerWrapper_CheckImmunity(BattlerWrapper *wrapper, uint16_t damageType);
bool BattlerWrapper_CheckVulnerability(BattlerWrapper *wrapper, uint16_t damageType);
bool BattlerWrapper_CheckResistance(BattlerWrapper *wrapper, uint16_t damageType);
bool BattlerWrapper_CheckAbsorption(BattlerWrapper *wrapper, uint16_t damageType);
bool BattlerWrapper_CheckStatusImmunity(BattlerWrapper *wrapper, uint16_t status);
uint16_t BattlerWrapper_GetAIType(BattlerWrapper *wrapper);
