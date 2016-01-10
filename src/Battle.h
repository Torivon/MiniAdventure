#pragma once
	
typedef struct BattleActor BattleActor;

bool ClosingWhileInBattle(void);
int GetCurrentMonsterHealth(void);
void SaveBattleState(void);

void ResumeBattle(int currentMonster);
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

