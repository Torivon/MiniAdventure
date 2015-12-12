#include <pebble.h>
#include "BattleActor.h"
#include "Character.h"
#include "Logging.h"
#include "NewBattle.h"
#include "Menu.h"
#include "Monsters.h"
#include "Skills.h"
#include "Story.h"
#include "BattleQueue.h"
#include "UILayers.h"

typedef struct NewBattleState
{
	uint16_t currentMonsterHealth;
	BattleActor *player;
	BattleActor *monster;
} NewBattleState;

static NewBattleState gBattleState = {0};
static MonsterDef *currentMonster = NULL;

bool gUpdateNewBattle = false;
bool gPlayerTurn = false;
int gSkillDelay = 0;
const char *gEffectDescription = NULL;

bool InNewBattle(void)
{
	return gUpdateNewBattle;
}

void CloseNewBattleWindow(void)
{
	INFO_LOG("Ending battle.");
	gUpdateNewBattle = false;
	ResetBattleQueue();
	PopMenu();
}

BattleActor *GetPlayerActor(void)
{
	return gBattleState.player;
}

BattleActor *GetMonsterActor(void)
{
	return gBattleState.monster;
}


void NewBattleWindowAppear(Window *window);
void NewBattleWindowDisappear(Window *window);
void NewBattleWindowInit(Window *window);

const char *FastAttackMenuText(void)
{
	if(gPlayerTurn)
	{
		return "Fast Attack";
	}
	
	return NULL;
}

const char *FastAttackMenuDescription(void)
{
	return "Quick stab";
}

void PlayerPushFastAttack(void)
{
	SkillInstance *newInstance = CreateSkillInstance(GetFastAttack(), GetPlayerActor(), GetMonsterActor());
	BattleQueuePush(SKILL, newInstance);
	BattleQueuePush(ACTOR, GetPlayerActor());
	gPlayerTurn = false;
}

const char *SlowAttackMenuText(void)
{
	if(gPlayerTurn)
	{
		return "Slow Attack";
	}
	
	return NULL;
}

const char *SlowAttackMenuDescription(void)
{
	return "Heavy slash";
}

void PlayerPushSlowAttack(void)
{
	SkillInstance *newInstance = CreateSkillInstance(GetSlowAttack(), GetPlayerActor(), GetMonsterActor());
	BattleQueuePush(SKILL, newInstance);
	BattleQueuePush(ACTOR, GetPlayerActor());
	gPlayerTurn = false;
}

const char *BattleEffectText(void)
{
	if(gSkillDelay > 0)
		return "Effect";
	else
		return NULL;
}

const char *BattleEffectDescription(void)
{
	return gEffectDescription;
}

MenuDefinition newBattleMainMenuDef = 
{
	.menuEntries = 
	{
		{.useFunctions = true, .textFunction = FastAttackMenuText, .descriptionFunction = FastAttackMenuDescription, .menuFunction = PlayerPushFastAttack},
		{.useFunctions = true, .textFunction = SlowAttackMenuText, .descriptionFunction = SlowAttackMenuDescription, .menuFunction = PlayerPushSlowAttack},
		{.useFunctions = true, .textFunction = BattleEffectText, .descriptionFunction = BattleEffectDescription, .menuFunction = DoNothing}
	},
	.init = NewBattleWindowInit,
	.appear = NewBattleWindowAppear,
	.disappear = NewBattleWindowDisappear,
	.disableBackButton = true,
	.mainImageId = -1,
	.floorImageId = -1
};

void ShowMainNewBattleMenu(void)
{
	INFO_LOG("Entering battle.");
	PushNewMenu(&newBattleMainMenuDef);
}

const char  *UpdateNewMonsterHealthText(void)
{
	static char monsterHealthText[] = "00000"; // Needs to be static because it's used by the system later.

	IntToString(monsterHealthText, 5, BattleActor_GetHealth(gBattleState.monster));
	return monsterHealthText;
}

void NewBattleWindowAppear(Window *window)
{
	MenuAppear(window);
	ShowMainWindowRow(0, currentMonster->name, UpdateNewMonsterHealthText());
	gUpdateNewBattle = true;
}

void NewBattleWindowDisappear(Window *window)
{
	MenuDisappear(window);
	gUpdateNewBattle = false;
}

int NewComputeMonsterHealth(int level)
{
	int baseHealth = 20 + ((level-1)*(level)/2) + ((level-1)*(level)*(level+1)/(6*2));
	return ScaleMonsterHealth(currentMonster, baseHealth);
}

void NewBattleInit(void)
{
	currentMonster = NULL;

	if(!currentMonster)
	{
		currentMonster = GetRandomMonster();
		gBattleState.currentMonsterHealth = NewComputeMonsterHealth(GetCurrentBaseLevel());
	}
	CharacterData *character = GetCharacter();
	gBattleState.player = InitBattleActor(true, character->level, character->speed, character->stats.maxHealth);
	gBattleState.monster = InitBattleActor(false, GetCurrentBaseLevel(), currentMonster->speed, gBattleState.currentMonsterHealth);
	
	BattleQueuePush(ACTOR, gBattleState.player);
	BattleQueuePush(ACTOR, gBattleState.monster);
	
	newBattleMainMenuDef.mainImageId = currentMonster->imageId;
#if defined(PBL_COLOR)
	newBattleMainMenuDef.floorImageId = RESOURCE_ID_IMAGE_BATTLE_FLOOR;
#endif
}

void UpdateNewBattle(void)
{
	bool entryRemoved = false;
	void *data = NULL;
	BattleQueueEntryType type = ACTOR;
	
	if(gSkillDelay > 0)
	{
		--gSkillDelay;
		return;
	}
	
	if(gPlayerTurn)
		return;
	
	if(BattleActor_GetHealth(gBattleState.player) <= 0 || BattleActor_GetHealth(gBattleState.monster) <= 0)
	{
		CloseNewBattleWindow();
		return;
	}
	
	entryRemoved = UpdateBattleQueue(&type, &data);
	
	if(entryRemoved)
	{
		switch(type)
		{
			case SKILL:
			{
				SkillInstance *instance = (SkillInstance*)data;
				gEffectDescription = ExecuteSkill(instance);
				gSkillDelay = SKILL_DELAY;
				break;
			}
			case ACTOR:
			{
				BattleActor *actor = (BattleActor*)data;
				if(BattleActor_IsPlayer(actor))
				{
					gPlayerTurn = true;
				}
				else
				{
					// Fake monster AI
					SkillInstance *newInstance = CreateSkillInstance(GetFastAttack(), GetMonsterActor(), GetPlayerActor());
					BattleQueuePush(SKILL, newInstance);
					BattleQueuePush(ACTOR, GetMonsterActor());
				}
				break;
			}
		}
	}

	ShowMainWindowRow(0, currentMonster->name, UpdateNewMonsterHealthText());
	UpdateHealthText(BattleActor_GetHealth(gBattleState.player), BattleActor_GetMaxHealth(gBattleState.player));
	RefreshMenuAppearance();
}

void NewBattleWindowInit(Window *window)
{
	MenuInit(window);
	NewBattleInit();
}

void ShowNewBattleWindow(void)
{
	if(CurrentLocationAllowsCombat())
		ShowMainNewBattleMenu();
}