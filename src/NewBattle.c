#include <pebble.h>
#include "AutoSizeConstants.h"
#include "Character.h"
#include "Clock.h"
#include "CombatantClass.h"
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

typedef struct NewBattleState
{
    NewBattleActor player;
    NewBattleActor monster;
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

static uint16_t maxTimeCount = 100;

static bool battleCleanExit = true;

static NewBattleState gBattleState;

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

bool gUpdateNewBattle = false;
bool gPlayerTurn = false;
bool gPlayerActed = false;
int gSkillDelay = 0;
const char *gEffectDescription = NULL;

void CloseNewBattleWindow(void)
{
    INFO_LOG("Ending battle.");
    battleCleanExit = true;
    PopGlobalState();
    Character_SetHealth(gBattleState.player.currentHealth);
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
    int count;
    for(count = 0; count < gBattleState.player.skillList.count; ++count)
    {
        if(gBattleState.player.skillList.entries[count].level > gBattleState.player.level)
            break;
    }
    return count;
}

static const char *BattleScreenNameCallback(int row)
{
    if(!gPlayerTurn)
        return NULL;
    
    SkillListEntry *entry = &gBattleState.player.skillList.entries[row];
    Skill *skill = ResourceBattler_GetPlayerSkillByID(entry->id);
    return skill->name;
}

static const char *BattleScreenDescriptionCallback(int row)
{
    if(!gPlayerTurn)
        return NULL;
    
    SkillListEntry *entry = &gBattleState.player.skillList.entries[row];
    if(entry->cooldown > 0)
        return "On cooldown";
    Skill *skill = ResourceBattler_GetPlayerSkillByID(entry->id);
    return skill->name;
}

static void BattleScreenSelectCallback(int row)
{
    if(!gPlayerTurn)
        return;
    
    if(gBattleState.player.skillList.entries[row].cooldown > 0)
        return;
    gBattleState.player.skillQueued = true;
    gBattleState.player.activeSkill = row;
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

static void InitializeNewBattleActor(NewBattleActor *actor, bool isPlayer, const char *name, CombatantClass *combatantClass, SkillList *skillList, uint16_t level, uint16_t currentHealth)
{
    actor->isPlayer = isPlayer;
    strncpy(actor->name, name, MAX_STORY_NAME_LENGTH);
    actor->combatantClass = *combatantClass;
    actor->skillList = *skillList;
    actor->level = level;
    actor->speed = CombatantClass_GetSpeed(combatantClass, level);
    actor->maxHealth = CombatantClass_GetHealth(combatantClass, level);
    actor->activeSkill = INVALID_SKILL;
    actor->counterSkill = INVALID_SKILL;
    if(currentHealth <= 0)
        actor->currentHealth = actor->maxHealth;
    else
        actor->currentHealth = currentHealth;
}

void NewBattleInit(void)
{
    ResourceMonster_UnloadCurrent();
    
    if(forcedBattle)
    {
        DEBUG_LOG("Starting forced battle with (%d,%d)", forcedBattleMonsterType, forcedBattleMonsterHealth);
        ResourceMonster_LoadCurrent(forcedBattleMonsterType);
        gBattleState.monster.currentHealth = forcedBattleMonsterHealth;
        forcedBattle = false;
    }
    
    if(!ResourceMonster_Loaded())
    {
        ResourceMonster_LoadCurrent(ResourceStory_GetCurrentLocationMonster());
    }
    InitializeNewBattleActor(&gBattleState.player, true, "Player", Character_GetCombatantClass(), Character_GetSkillList(), Character_GetLevel(), Character_GetHealth());
    InitializeNewBattleActor(&gBattleState.monster, false, ResourceMonster_GetCurrentName(), ResourceStory_GetCurrentMonsterCombatantClass(), ResourceStory_GetCurrentMonsterSkillList(), ResourceStory_GetCurrentLocationBaseLevel(), 0);
    
    playerHealthBar = CreateProgressBar(&gBattleState.player.currentHealth, &gBattleState.player.maxHealth, FILL_UP, playerHealthFrame, GColorBrightGreen, -1);
    monsterHealthBar = CreateProgressBar(&gBattleState.monster.currentHealth, &gBattleState.monster.maxHealth, FILL_DOWN, monsterHealthFrame, GColorFolly, -1);
    playerTimeBar = CreateProgressBar(&gBattleState.player.currentTime, &maxTimeCount, FILL_UP, playerTimeFrame, GColorVeryLightBlue, -1);
    monsterTimeBar = CreateProgressBar(&gBattleState.monster.currentTime, &maxTimeCount, FILL_DOWN, monsterTimeFrame, GColorRichBrilliantLavender, -1);
    
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

static void DoSkill(Skill *skill, NewBattleActor *attacker, NewBattleActor *defender)
{
    gEffectDescription = ExecuteSkill(skill, attacker, defender);
    MarkProgressBarDirty(playerHealthBar);
    MarkProgressBarDirty(monsterHealthBar);
    SetDescription(gEffectDescription);
    gSkillDelay = SKILL_DELAY;
    attacker->currentTime = 0;
    MarkProgressBarDirty(playerTimeBar);
    MarkProgressBarDirty(monsterTimeBar);
    attacker->skillQueued = false;
}

void UpdateNewBattle(void *unused)
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
    
    if(gBattleState.player.currentHealth <= 0 || gBattleState.monster.currentHealth <= 0)
    {
        CloseNewBattleWindow();
        return;
    }
    
    gBattleState.player.currentTime += gBattleState.player.speed;
    gBattleState.monster.currentTime += gBattleState.monster.speed;
    
    MarkProgressBarDirty(playerTimeBar);
    MarkProgressBarDirty(monsterTimeBar);
    
    bool playerAhead = gBattleState.player.currentTime >= gBattleState.monster.currentTime;
    
    if(playerAhead)
    {
        if(gBattleState.player.currentTime >= maxTimeCount)
        {
            if(gBattleState.player.skillQueued)
            {
                Skill *skill = ResourceStory_GetLoadedSkillByID(true, gBattleState.player.skillList.entries[gBattleState.player.activeSkill].id);
                DoSkill(skill, &gBattleState.player, &gBattleState.monster);
            }
            else
            {
                UpdateSkillCooldowns(&gBattleState.player.skillList);
                SetDescription("Your turn");
                gPlayerTurn = true;
                gPlayerActed = false;
                ReloadMenu(GetMainMenu());
           }
        }
    }
    else
    {
        if(gBattleState.monster.currentTime >= maxTimeCount)
        {
            if(gBattleState.monster.skillQueued)
            {
                Skill *skill = ResourceStory_GetLoadedSkillByID(false, gBattleState.monster.skillList.entries[gBattleState.monster.activeSkill].id);
                DoSkill(skill, &gBattleState.monster, &gBattleState.player);
            }
            else
            {
                UpdateSkillCooldowns(&gBattleState.monster.skillList);
                gBattleState.monster.activeSkill = 0;
                gBattleState.monster.skillQueued = true;
                gBattleState.monster.currentTime = 0;
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
