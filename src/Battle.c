#include <pebble.h>
#include "AutoSizeConstants.h"
#include "BaseWindow.h"
#include "Battle.h"
#include "Character.h"
#include "Clock.h"
#include "CombatantClass.h"
#include "DescriptionFrame.h"
#include "GlobalState.h"
#include "Logging.h"
#include "MainImage.h"
#include "Menu.h"
#include "Persistence.h"
#include "ProgressBar.h"
#include "ResourceStory.h"
#include "Skills.h"

typedef struct BattleState
{
    BattleActorWrapper player;
    BattleActorWrapper monster;
} BattleState;

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

static uint16_t maxTimeCount = 100;

static bool battleCleanExit = true;

static BattleState gBattleState;

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

bool gUpdateBattle = false;
bool gPlayerTurn = false;
bool gPlayerActed = false;
int gSkillDelay = 0;
const char *gEffectDescription = NULL;

void CloseBattleWindow(void)
{
    INFO_LOG("Ending battle.");
    battleCleanExit = true;
    PopGlobalState();
    Character_SetHealth(gBattleState.player.actor.currentHealth);
    Character_SetCooldowns(gBattleState.player.actor.skillCooldowns);
    ShowDateLayer();
}

bool ClosingWhileInBattle(void)
{
    return !battleCleanExit;
}

static uint16_t BattleScreenCount(void)
{
    DEBUG_LOG("BattleScreenCount");
    if(!gPlayerTurn)
        return 0;
    
    DEBUG_LOG("Returning good value");
    return BattlerWrapper_GetUsableSkillCount(gBattleState.player.battlerWrapper, gBattleState.player.actor.level);
}

static const char *BattleScreenNameCallback(int row)
{
    if(!gPlayerTurn)
        return NULL;
    
    Skill *skill = BattlerWrapper_GetSkillByIndex(gBattleState.player.battlerWrapper, row);
    return skill->name;
}

static const char *BattleScreenDescriptionCallback(int row)
{
    if(!gPlayerTurn)
        return NULL;
    
    if(gBattleState.player.actor.skillCooldowns[row] > 0)
        return "On cooldown";
    Skill *skill = BattlerWrapper_GetSkillByIndex(gBattleState.player.battlerWrapper, row);
    return skill->name;
}

static void BattleScreenSelectCallback(int row)
{
    if(!gPlayerTurn)
        return;
    
    if(gBattleState.player.actor.skillCooldowns[row] > 0)
        return;
    gBattleState.player.actor.skillQueued = true;
    gBattleState.player.actor.activeSkill = row;
    gPlayerActed = true;
}

void BattleScreenAppear(void *data)
{
    if(gPlayerActed)
    {
        gPlayerTurn = false;
        gPlayerActed = false;
    }
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

static void InitializeBattleActorWrapper(BattleActorWrapper *actorWrapper, BattlerWrapper *battlerWrapper,uint16_t level, uint16_t currentHealth, uint16_t *skillCooldowns)
{
    actorWrapper->battlerWrapper = battlerWrapper;
    actorWrapper->actor.level = level;
    actorWrapper->actor.maxHealth = CombatantClass_GetHealth(BattlerWrapper_GetCombatantClass(battlerWrapper), level);
    actorWrapper->actor.skillQueued = false;
    actorWrapper->actor.activeSkill = INVALID_SKILL;
    actorWrapper->actor.counterSkill = INVALID_SKILL;
    actorWrapper->actor.currentTime = 0;
    if(currentHealth == 0)
        actorWrapper->actor.currentHealth = actorWrapper->actor.maxHealth;
    else
        actorWrapper->actor.currentHealth = currentHealth;
    for(int i = 0; i < MAX_SKILLS_IN_LIST; ++i)
    {
        actorWrapper->actor.skillCooldowns[i] = skillCooldowns[i];
    }
}

void BattleInit(void)
{
    ResourceMonster_UnloadCurrent();
    
    if(forcedBattle)
    {
        DEBUG_LOG("Starting forced battle with (%d,%d)", forcedBattleMonsterType, forcedBattleMonsterHealth);
        ResourceMonster_LoadCurrent(forcedBattleMonsterType);
        gBattleState.monster.actor.currentHealth = forcedBattleMonsterHealth;
        forcedBattle = false;
    }
    
    if(!ResourceMonster_Loaded())
    {
        ResourceMonster_LoadCurrent(ResourceStory_GetCurrentLocationMonster());
    }
    InitializeBattleActorWrapper(&gBattleState.player, BattlerWrapper_GetPlayerWrapper(), Character_GetLevel(), Character_GetHealth(), Character_GetCooldowns());
    uint16_t skillCooldowns[MAX_SKILLS_IN_LIST] = {0};
    InitializeBattleActorWrapper(&gBattleState.monster, BattlerWrapper_GetMonsterWrapper(), ResourceStory_GetCurrentLocationBaseLevel(), 0, skillCooldowns);
    
    playerHealthBar = CreateProgressBar(&gBattleState.player.actor.currentHealth, &gBattleState.player.actor.maxHealth, FILL_UP, playerHealthFrame, GColorBrightGreen, -1);
    monsterHealthBar = CreateProgressBar(&gBattleState.monster.actor.currentHealth, &gBattleState.monster.actor.maxHealth, FILL_DOWN, monsterHealthFrame, GColorFolly, -1);
    playerTimeBar = CreateProgressBar(&gBattleState.player.actor.currentTime, &maxTimeCount, FILL_UP, playerTimeFrame, GColorVeryLightBlue, -1);
    monsterTimeBar = CreateProgressBar(&gBattleState.monster.actor.currentTime, &maxTimeCount, FILL_DOWN, monsterTimeFrame, GColorRichBrilliantLavender, -1);
    
    InitializeProgressBar(playerHealthBar, GetBaseWindow());
    InitializeProgressBar(playerTimeBar, GetBaseWindow());
    InitializeProgressBar(monsterHealthBar, GetBaseWindow());
    InitializeProgressBar(monsterTimeBar, GetBaseWindow());
    
    HideDateLayer();
    
    // Force the main menu to the front
    InitializeMenuLayer(GetMainMenu(), GetBaseWindow());
    InitializeMenuLayer(GetSlaveMenu(), GetBaseWindow());
    
    RegisterMenuCellCallbacks(GetMainMenu(), BattleScreenCount, BattleScreenNameCallback, BattleScreenDescriptionCallback, BattleScreenSelectCallback);
    
    DEBUG_VERBOSE_LOG("Finished battle init");
    battleCleanExit = false;
}

static void DoSkill(Skill *skill, BattleActorWrapper *attacker, BattleActorWrapper *defender)
{
    gEffectDescription = ExecuteSkill(skill, attacker, defender);
    MarkProgressBarDirty(playerHealthBar);
    MarkProgressBarDirty(monsterHealthBar);
    SetDescription(gEffectDescription);
    gSkillDelay = SKILL_DELAY;
    attacker->actor.currentTime = 0;
    MarkProgressBarDirty(playerTimeBar);
    MarkProgressBarDirty(monsterTimeBar);
    attacker->actor.skillQueued = false;
}

static void UpdateActorCurrentTime(BattleActorWrapper *wrapper)
{
    if(wrapper->actor.skillQueued)
    {
        wrapper->actor.currentTime += BattlerWrapper_GetSkillByIndex(wrapper->battlerWrapper, wrapper->actor.activeSkill)->speed;
    }
    else
    {
        wrapper->actor.currentTime += CombatantClass_GetSpeed(BattlerWrapper_GetCombatantClass(wrapper->battlerWrapper), wrapper->actor.level);
    }
}

void UpdateBattle(void *unused)
{
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
    
    if(gBattleState.player.actor.currentHealth <= 0 || gBattleState.monster.actor.currentHealth <= 0)
    {
        CloseBattleWindow();
        return;
    }
    
    UpdateActorCurrentTime(&gBattleState.player);
    UpdateActorCurrentTime(&gBattleState.monster);
    
    MarkProgressBarDirty(playerTimeBar);
    MarkProgressBarDirty(monsterTimeBar);
    
    bool playerAhead = gBattleState.player.actor.currentTime >= gBattleState.monster.actor.currentTime;
    
    if(playerAhead)
    {
        if(gBattleState.player.actor.currentTime >= maxTimeCount)
        {
            if(gBattleState.player.actor.skillQueued)
            {
                Skill *skill = BattlerWrapper_GetSkillByIndex(gBattleState.player.battlerWrapper, gBattleState.player.actor.activeSkill);
                DoSkill(skill, &gBattleState.player, &gBattleState.monster);
            }
            else
            {
                UpdateSkillCooldowns(gBattleState.player.actor.skillCooldowns);
                SetDescription("Your turn");
                gPlayerTurn = true;
                gPlayerActed = false;
                ReloadMenu(GetMainMenu());
                gBattleState.player.actor.currentTime = 0;
           }
        }
    }
    else
    {
        if(gBattleState.monster.actor.currentTime >= maxTimeCount)
        {
            if(gBattleState.monster.actor.skillQueued)
            {
                Skill *skill = BattlerWrapper_GetSkillByIndex(gBattleState.monster.battlerWrapper, gBattleState.monster.actor.activeSkill);
                DoSkill(skill, &gBattleState.monster, &gBattleState.player);
            }
            else
            {
                UpdateSkillCooldowns(gBattleState.monster.actor.skillCooldowns);
                gBattleState.monster.actor.activeSkill = 0;
                gBattleState.monster.actor.skillQueued = true;
                gBattleState.monster.actor.currentTime = 0;
            }
        }
    }
}

void BattleScreenPush(void *data)
{
    BattleInit();
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
        PushGlobalState(STATE_BATTLE, SECOND_UNIT, UpdateBattle, BattleScreenPush, BattleScreenAppear, NULL, BattleScreenPop, NULL);
}
