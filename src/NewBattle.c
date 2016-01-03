#include <pebble.h>
#include "BattleActor.h"
#include "Character.h"
#include "Clock.h"
#include "DescriptionFrame.h"
#include "GlobalState.h"
#include "Logging.h"
#include "NewBattle.h"
#include "MainImage.h"
#include "NewBaseWindow.h"
#include "NewMenu.h"
#include "Persistence.h"
#include "ProgressBar.h"
#include "ResourceStory.h"
#include "Skills.h"
#include "BattleQueue.h"

typedef struct NewBattleState
{
    uint16_t currentMonsterHealth;
    BattleActor *player;
    BattleActor *monster;
    int playerCurrentHealth;
    int playerMaxHealth;
    int monsterCurrentHealth;
    int monsterMaxHealth;
    int playerTime;
    int monsterTime;
} NewBattleState;

#if defined(PBL_ROUND)
static GRect playerHealthFrame = {.origin = {.x = 50, .y = 90}, .size = {.w = 16, .h = 40}};
static GRect playerTimeFrame = {.origin = {.x = 67, .y = 90}, .size = {.w = 8, .h = 40}};
static GRect monsterHealthFrame = {.origin = {.x = 50, .y = 48}, .size = {.w = 16, .h = 40}};
static GRect monsterTimeFrame = {.origin = {.x = 67, .y = 48}, .size = {.w = 8, .h = 40}};
#else
static GRect playerHealthFrame = {.origin = {.x = 20, .y = 65}, .size = {.w = 16, .h = 40}};
static GRect playerTimeFrame = {.origin = {.x = 36, .y = 65}, .size = {.w = 8, .h = 40}};
static GRect monsterHealthFrame = {.origin = {.x = 148, .y = 65}, .size = {.w = 16, .h = 40}};
static GRect monsterTimeFrame = {.origin = {.x = 140, .y = 65}, .size = {.w = 8, .h = 40}};
#endif


static ProgressBar *playerHealthBar;
static ProgressBar *playerTimeBar;

static ProgressBar *monsterHealthBar;
static ProgressBar *monsterTimeBar;

static int maxTimeCount = 100;

static bool battleCleanExit = true;

static NewBattleState gBattleState = {0};

// in seconds
#define BATTLE_SAVE_TICK_DELAY 60
static int battleSaveTickDelay = 0;

void SaveBattleState(void)
{
    if(battleSaveTickDelay <= 0)
    {
        //SavePersistedData();
        battleSaveTickDelay = BATTLE_SAVE_TICK_DELAY;
    }
}

int GetCurrentMonsterHealth(void)
{
    return gBattleState.currentMonsterHealth;
}

bool gUpdateNewBattle = false;
bool gPlayerTurn = false;
int gSkillDelay = 0;
const char *gEffectDescription = NULL;

void CloseNewBattleWindow(void)
{
    INFO_LOG("Ending battle.");
    battleCleanExit = true;
    ResetBattleQueue();
    PopGlobalState();
    Character_SetHealth(BattleActor_GetHealth(GetPlayerActor()));
    ShowDateLayer();
}

bool ClosingWhileInBattle(void)
{
    return !battleCleanExit;
}

BattleActor *GetPlayerActor(void)
{
    return gBattleState.player;
}

BattleActor *GetMonsterActor(void)
{
    return gBattleState.monster;
}

static uint16_t BattleScreenCount(void)
{
    DEBUG_LOG("BattleScreenCount");
    if(!gPlayerTurn)
        return 0;
    
    DEBUG_LOG("Returning good value");
    return BattleActor_GetSkillList(gBattleState.player)->count;
}

static const char *BattleScreenNameCallback(int row)
{
    if(!gPlayerTurn)
        return NULL;
    
    SkillListEntry *entry = &BattleActor_GetSkillList(gBattleState.player)->entries[row];
    Skill *skill = GetSkillByID(entry->id);
    return GetSkillName(skill);
}

static const char *BattleScreenDescriptionCallback(int row)
{
    if(!gPlayerTurn)
        return NULL;
    
    SkillListEntry *entry = &BattleActor_GetSkillList(gBattleState.player)->entries[row];
    if(entry->cooldown > 0)
        return "On cooldown";
    Skill *skill = GetSkillByID(entry->id);
    return GetSkillName(skill);
}

static void BattleScreenSelectCallback(int row)
{
    if(!gPlayerTurn)
        return;
    
    SkillList *skillList = BattleActor_GetSkillList(gBattleState.player);
    SkillListEntry *entry = &skillList->entries[row];
    if(entry->cooldown > 0)
        return;
    SkillInstance *newInstance = CreateSkillInstance(&skillList->entries[row], GetPlayerActor(), GetMonsterActor());
    BattleQueuePush(SKILL, newInstance);
    gPlayerTurn = false;
}

const char  *UpdateNewMonsterHealthText(void)
{
    static char monsterHealthText[] = "00000"; // Needs to be static because it's used by the system later.
    
    IntToString(monsterHealthText, 5, BattleActor_GetHealth(GetMonsterActor()));
    return monsterHealthText;
}

void BattleScreenAppear(void *data)
{
    SetDescription(ResourceMonster_GetCurrentName());
    RegisterMenuCellCallbacks(GetMainMenu(), BattleScreenCount, BattleScreenNameCallback, BattleScreenDescriptionCallback, BattleScreenSelectCallback);
    ReloadMenu(GetMainMenu());
    SetForegroundImage(ResourceStory_GetCurrentMonsterImage());
#if defined(PBL_COLOR)
    SetBackgroundImage(RESOURCE_ID_IMAGE_BATTLE_FLOOR);
#endif
    SetMainImageVisibility(true, true, true);
}

static bool forcedBattle = false;
static int forcedBattleMonsterType = -1;
static int forcedBattleMonsterHealth = 0;
void ResumeBattle(int currentMonster, int currentMonsterHealth)
{
    if(currentMonster >= 0 && currentMonsterHealth > 0)
    {
        forcedBattle = true;
        forcedBattleMonsterType = currentMonster;
        forcedBattleMonsterHealth = currentMonsterHealth;
    }
}

bool IsBattleForced(void)
{
    return forcedBattle;
}

void NewBattleInit(void)
{
    ResourceMonster_UnloadCurrent();
    
    if(forcedBattle)
    {
        DEBUG_LOG("Starting forced battle with (%d,%d)", forcedBattleMonsterType, forcedBattleMonsterHealth);
        ResourceMonster_LoadCurrent(forcedBattleMonsterType);
        gBattleState.currentMonsterHealth = forcedBattleMonsterHealth;
        forcedBattle = false;
    }
    
    if(!ResourceMonster_Loaded())
    {
        ResourceMonster_LoadCurrent(ResourceStory_GetCurrentLocationMonster());
    }
    gBattleState.player = BattleActor_Init(true, Character_GetCombatantClass(), Character_GetSkillList(), Character_GetLevel(), Character_GetHealth());
    gBattleState.monster = BattleActor_Init(false, ResourceStory_GetCurrentMonsterCombatantClass(), ResourceStory_GetCurrentMonsterSkillList(), ResourceStory_GetCurrentLocationBaseLevel(), 0);
    
    BattleQueuePush(ACTOR, GetPlayerActor());
    BattleQueuePush(ACTOR, GetMonsterActor());
    
    playerHealthBar = CreateProgressBar(&gBattleState.playerCurrentHealth, &gBattleState.playerMaxHealth, FILL_UP, playerHealthFrame, GColorBrightGreen, -1);
    monsterHealthBar = CreateProgressBar(&gBattleState.monsterCurrentHealth, &gBattleState.monsterMaxHealth, FILL_DOWN, monsterHealthFrame, GColorFolly, -1);
    playerTimeBar = CreateProgressBar(&gBattleState.playerTime, &maxTimeCount, FILL_UP, playerTimeFrame, GColorVeryLightBlue, -1);
    monsterTimeBar = CreateProgressBar(&gBattleState.monsterTime, &maxTimeCount, FILL_DOWN, monsterTimeFrame, GColorRichBrilliantLavender, -1);
    
    InitializeProgressBar(playerHealthBar, GetBaseWindow());
    InitializeProgressBar(playerTimeBar, GetBaseWindow());
    InitializeProgressBar(monsterHealthBar, GetBaseWindow());
    InitializeProgressBar(monsterTimeBar, GetBaseWindow());
    
    HideDateLayer();
    
    // Force the main menu to the front
    InitializeNewMenuLayer(GetMainMenu(), GetBaseWindow());
    InitializeNewMenuLayer(GetSlaveMenu(), GetBaseWindow());
    
    RegisterMenuCellCallbacks(GetMainMenu(), BattleScreenCount, BattleScreenNameCallback, BattleScreenDescriptionCallback, BattleScreenSelectCallback);
    
    DEBUG_VERBOSE_LOG("Finished battle init");
    battleCleanExit = false;
}

void UpdateProgressBars(void)
{
    gBattleState.playerCurrentHealth = BattleActor_GetHealth(GetPlayerActor());
    gBattleState.playerMaxHealth = BattleActor_GetMaxHealth(GetPlayerActor());
    gBattleState.monsterCurrentHealth = BattleActor_GetHealth(GetMonsterActor());
    gBattleState.monsterMaxHealth = BattleActor_GetMaxHealth(GetMonsterActor());
    
    gBattleState.playerTime = GetCurrentTimeInQueue(true);
    gBattleState.monsterTime = GetCurrentTimeInQueue(false);
}

void UpdateNewBattle(void *unused)
{
    bool entryRemoved = false;
    void *data = NULL;
    BattleQueueEntryType type = ACTOR;
    
    if(battleSaveTickDelay > 0)
    {
        --battleSaveTickDelay;
    }
    
    if(gSkillDelay > 0)
    {
        --gSkillDelay;
        if(gSkillDelay == 0)
            SetDescription(ResourceMonster_GetCurrentName());
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
    
    UpdateProgressBars();
    MarkProgressBarDirty(playerTimeBar);
    MarkProgressBarDirty(monsterTimeBar);
    
    if(entryRemoved)
    {
        switch(type)
        {
            case SKILL:
            {
                SkillInstance *instance = (SkillInstance*)data;
                gEffectDescription = ExecuteSkill(instance);
                UpdateProgressBars();
                MarkProgressBarDirty(playerHealthBar);
                MarkProgressBarDirty(monsterHealthBar);
                SetDescription(gEffectDescription);
                BattleQueuePush(ACTOR, SkillInstanceGetAttacker(instance));
                gSkillDelay = SKILL_DELAY;
                break;
            }
            case ACTOR:
            {
                BattleActor *actor = (BattleActor*)data;
                UpdateSkillCooldowns(BattleActor_GetSkillList(actor));
                if(BattleActor_IsPlayer(actor))
                {
                    SetDescription("Your turn");
                    gPlayerTurn = true;
                    ReloadMenu(GetMainMenu());
                }
                else
                {
                    // Fake monster AI
                    SkillList *skillList = BattleActor_GetSkillList(gBattleState.monster);
                    SkillInstance *newInstance = CreateSkillInstance(&skillList->entries[0], GetMonsterActor(), GetPlayerActor());
                    BattleQueuePush(SKILL, newInstance);
                }
                break;
            }
        }
    }
}

void BattleScreenPush(void *data)
{
    NewBattleInit();
}

void BattleScreenPop(void *data)
{
    RemoveProgressBar(playerHealthBar);
    RemoveProgressBar(playerTimeBar);
    RemoveProgressBar(monsterHealthBar);
    RemoveProgressBar(monsterTimeBar);
    FreeProgressBar(playerHealthBar);
    FreeProgressBar(playerTimeBar);
    FreeProgressBar(monsterHealthBar);
    FreeProgressBar(monsterTimeBar);
    ResourceMonster_UnloadCurrent();
}

void TriggerBattleScreen(void)
{
    if(ResourceStory_CurrentLocationHasMonster())
        PushGlobalState(STATE_BATTLE, SECOND_UNIT, UpdateNewBattle, BattleScreenPush, BattleScreenAppear, NULL, BattleScreenPop, NULL);
}
