#pragma once
	
typedef struct BattleActor BattleActor;

void ShowNewBattleWindow(void);

bool ClosingWhileInNewBattle(void);

void ResumeBattle(int currentMonster, int currentMonsterHealth);
bool IsBattleForced(void);

void UpdateNewBattle(void);
bool InNewBattle(void);

BattleActor *GetPlayerActor(void);
BattleActor *GetMonsterActor(void);
