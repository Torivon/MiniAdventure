#pragma once
	
typedef struct BattleActor BattleActor;

bool ClosingWhileInBattle(void);
int GetCurrentMonsterHealth(void);
void SaveBattleState(void);

void ResumeBattle(int currentMonster, int currentMonsterHealth);
bool IsBattleForced(void);

void UpdateNewBattle(void);

BattleActor *GetPlayerActor(void);
BattleActor *GetMonsterActor(void);
void TriggerBattleScreen(void);
void ResumeBattle(int currentMonster, int currentMonsterHealth);
bool IsBattleForced(void);
bool ClosingWhileInBattle(void);
