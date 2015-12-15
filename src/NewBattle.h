#pragma once
	
typedef struct BattleActor BattleActor;

bool ClosingWhileInNewBattle(void);

void ResumeBattle(int currentMonster, int currentMonsterHealth);
bool IsBattleForced(void);

void UpdateNewBattle(void);

BattleActor *GetPlayerActor(void);
BattleActor *GetMonsterActor(void);
void TriggerBattleScreen(void);
