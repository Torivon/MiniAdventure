#pragma once
	
typedef struct BattleActor BattleActor;

bool ClosingWhileInBattle(void);
int GetCurrentMonsterHealth(void);
void SaveBattleState(void);

void Battle_SetCleanExit(void);
void ResumeBattle(int currentMonster);
void ForceRandomBattle(void);
bool IsBattleForced(void);

BattleActor *GetPlayerActor(void);
BattleActor *GetMonsterActor(void);
void TriggerBattleScreen(void);
bool IsBattleForced(void);
bool ClosingWhileInBattle(void);
void BattleScreenPop(void *data);

uint16_t Battle_GetCurrentMonsterIndex(void);
void Battle_WritePlayerData(int index);
void Battle_WriteMonsterData(int index);
void Battle_ReadPlayerData(int index);
void Battle_ReadMonsterData(int index);

void BattleScreenPush(void *data);
void BattleScreenPop(void *data);
void UpdateBattle(void *unused);
void BattleScreenAppear(void *data);

uint16_t BattleScreen_MenuSectionCount(void);
const char *BattleScreen_MenuSectionName(uint16_t sectionIndex);
uint16_t BattleScreen_MenuCellCount(uint16_t sectionIndex);
const char *BattleScreen_MenuCellName(MenuIndex *index);
const char *BattleScreen_MenuCellDescription(MenuIndex *index);
void BattleScreen_MenuSelect(MenuIndex *index);

void Battle_InitializeNewMonster(uint16_t monsterIndex, bool fullHeal);
