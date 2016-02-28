#include <pebble.h>
#include "AI.h"
#include "AutoSizeConstants.h"
#include "AutoSkillConstants.h"
#include "BaseWindow.h"
#include "Battle.h"
#include "BattleEvents.h"
#include "Battler.h"
#include "BinaryResourceLoading.h"
#include "Character.h"
#include "Clock.h"
#include "CombatantClass.h"
#include "DescriptionFrame.h"
#include "DialogFrame.h"
#include "EngineInfo.h"
#include "EngineMenu.h"
#include "GlobalState.h"
#include "Location.h"
#include "Logging.h"
#include "MainImage.h"
#include "Menu.h"
#include "Persistence.h"
#include "ProgressBar.h"
#include "Skills.h"

typedef struct BattleState
{
    BattleActorWrapper player;
    BattleActorWrapper monster;
} BattleState;

static ProgressBar *playerHealthBar = NULL;
static ProgressBar *playerTimeBar = NULL;

static ProgressBar *monsterHealthBar = NULL;
static ProgressBar *monsterTimeBar = NULL;

static uint16_t maxTimeCount = 100;

static bool battleCleanExit = true;

static BattleState gBattleState =
{
    .player =
    {
        .actor = {0},
        .battlerWrapper = NULL
    },
    .monster =
    {
        .actor = {0},
        .battlerWrapper = NULL
    }
};

BattleActor *GetMonsterActor(void)
{
    return &gBattleState.monster.actor;
}

BattleActor *GetPlayerActor(void)
{
    return &gBattleState.player.actor;
}

BattleActorWrapper *GetMonsterActorWrapper(void)
{
    return &gBattleState.monster;
}

BattleActorWrapper *GetPlayerActorWrapper(void)
{
    return &gBattleState.player;
}

static uint16_t currentMonsterIndex = 0;

uint16_t Battle_GetCurrentMonsterIndex(void)
{
    return currentMonsterIndex;
}

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

void Battle_WritePlayerData(int index)
{
    persist_write_data(index, &gBattleState.player.actor, sizeof(BattleActor));
}

void Battle_WriteMonsterData(int index)
{
    persist_write_data(index, &gBattleState.monster.actor, sizeof(BattleActor));
}

void Battle_ReadPlayerData(int index)
{
    persist_read_data(index, &gBattleState.player.actor, sizeof(BattleActor));
}

void Battle_ReadMonsterData(int index)
{
    persist_read_data(index, &gBattleState.monster.actor, sizeof(BattleActor));
}

static bool gPlayerTurn = false;
static bool gPlayerActed = false;
static const char *gEffectDescription = NULL;

void CloseBattleWindow(void)
{
    INFO_LOG("Ending battle.");
    battleCleanExit = true;
    Character_SetHealth(gBattleState.player.actor.currentHealth);
    if(gBattleState.player.actor.currentHealth > 0)
    {
        // The player won, so grant xp.
        bool leveledUp = Character_GrantXP(gBattleState.monster.actor.level);
        BattlerWrapper_TriggerEvent(gBattleState.monster.battlerWrapper);
        EngineInfo_QueueWinDialog();
        if(leveledUp)
        {
            EngineInfo_QueueLevelUpDialog();
        }
    }
    
    GlobalState_Pop();
    Character_SetCooldowns(gBattleState.player.actor.skillCooldowns);
}

bool ClosingWhileInBattle(void)
{
    return !battleCleanExit;
}

void Battle_SetCleanExit(void)
{
    battleCleanExit = true;
}

uint16_t BattleScreen_MenuSectionCount(void)
{
    return 3 + EngineMenu_GetSectionCount();
}

const char *BattleScreen_MenuSectionName(uint16_t sectionIndex)
{
    switch(sectionIndex)
    {
        case 0:
            return "Skills";
        case 1:
            return "Events";
        case 2:
            return "Battle";
        case 3:
            return EngineMenu_GetSectionName();
    }
    return "None";
}

uint16_t BattleScreen_MenuCellCount(uint16_t sectionIndex)
{
    switch(sectionIndex)
    {
        case 0:
        {
            if(!gPlayerTurn)
                return 0;
            
            return BattlerWrapper_GetUsableSkillCount(gBattleState.player.battlerWrapper, gBattleState.player.actor.level);
        }
        case 1:
        {
            if(!gPlayerTurn)
                return 0;
            
            return BattleEvent_GetCurrentAvailableBattleEvents();
        }
        case 2:
        {
            return 4;
        }
        case 3:
        {
            return EngineMenu_GetCellCount();
        }
    }
    return 0;
}

const char *BattleScreen_MenuCellName(MenuIndex *index)
{
    switch(index->section)
    {
        case 0:
        {
            if(!gPlayerTurn)
                return NULL;
            
            Skill *skill = BattlerWrapper_GetSkillByIndex(gBattleState.player.battlerWrapper, index->row);
            return skill->name;
        }
        case 1:
        {
            if(!gPlayerTurn)
                return NULL;

            return BattleEvent_GetCurrentBattleEventName(index->row);
        }
        case 2:
        {
            switch(index->row)
            {
                case 0:
                    return "Status";
                case 1:
                    return "Class";
                case 2:
                    return "Skills";
                case 3:
                    return "Reset";
            }
            break;
        }
        case 3:
        {
            return EngineMenu_GetCellName(index->row);
        }
    }
    return "None";
}

const char *BattleScreen_MenuCellDescription(MenuIndex *index)
{
    switch(index->section)
    {
        case 0:
        {
            if(!gPlayerTurn)
                return NULL;
            
            if(gBattleState.player.actor.skillCooldowns[index->row] > 0)
                return "On cooldown";
            Skill *skill = BattlerWrapper_GetSkillByIndex(gBattleState.player.battlerWrapper, index->row);
            return skill->name;
        }
        case 1:
        {
            if(!gPlayerTurn)
                return NULL;

            return BattleEvent_GetCurrentBattleEventDescription(index->row);
        }
        case 2:
        {
            switch(index->row)
            {
                case 0:
                    return "Status";
                case 1:
                    return "Class";
                case 2:
                    return "Skills";
                case 3:
                    return "Reset";
            }
            break;
        }
        case 3:
        {
            return EngineMenu_GetCellName(index->row);
        }
    }
    return "None";
}

void BattleScreen_MenuSelect(MenuIndex *index)
{
    switch(index->section)
    {
        case 0:
        {
            if(!gPlayerTurn)
                return;
            
            if(gBattleState.player.actor.skillCooldowns[index->row] > 0)
                return;
            Skill *skill = BattlerWrapper_GetSkillByIndex(gBattleState.player.battlerWrapper, index->row);
            if(gBattleState.player.actor.statusEffectDurations[STATUS_EFFECT_SILENCE] > 0 ||
               gBattleState.player.actor.statusEffectDurations[STATUS_EFFECT_PASSIFY] > 0)
            {
                if((skill->damageType & DAMAGE_TYPE_MAGIC) && gBattleState.player.actor.statusEffectDurations[STATUS_EFFECT_SILENCE])
                    return;
                if((skill->damageType & DAMAGE_TYPE_PHYSICAL) && gBattleState.player.actor.statusEffectDurations[STATUS_EFFECT_PASSIFY])
                    return;
            }
            gBattleState.player.actor.skillQueued = true ;
            gBattleState.player.actor.activeSkill = index->row;
            gBattleState.player.actor.skillCooldowns[gBattleState.player.actor.activeSkill] = GetSkillCooldown(skill);
            gPlayerActed = true;
            break;
        }
        case 1:
        {
            if(!gPlayerTurn)
                return;

            BattleEvent_MenuQueue(index->row);
            gPlayerActed = true;
            break;
        }
        case 2:
        {
            switch(index->row)
            {
                case 0:
                {
                    Character_ShowStatus();
                    break;
                }
                case 1:
                {
                    Character_ShowClass();
                    break;
                }
                case 2:
                {
                    Character_ShowSkills();
                    break;
                }
                case 3:
                {
                    EngineInfo_QueueResetDialog();
                    GlobalState_QueueStatePop();
                    GlobalState_Queue(STATE_RESET_GAME, 0, NULL);
                    break;
                }
            }
            break;
        }
        case 3:
        {
            EngineMenu_SelectAction(index->row);
        }
    }
}

void BattleScreenAppear(void *data)
{
    if(!gBattleState.monster.battlerWrapper)
        return;
    
    if(gPlayerActed)
    {
        gPlayerTurn = false;
        gPlayerActed = false;
        gBattleState.player.actor.currentTime = 0;
    }
    SetDescription(Monster_GetCurrentName());
    RegisterMenuState(GetMainMenu(), STATE_BATTLE);
    RegisterMenuState(GetSlaveMenu(), STATE_NONE);
    SetForegroundImage(Monster_GetCurrentImage());
#if defined(PBL_COLOR)
    SetBackgroundImage(RESOURCE_ID_IMAGE_BATTLEFLOOR);
#endif
    SetMainImageVisibility(true, true, true);
}

static bool forcedBattle = false;
static int forcedBattleMonsterType = -1;
void ResumeBattle(int currentMonster)
{
    forcedBattle = true;
    forcedBattleMonsterType = currentMonster;
}

void ForceRandomBattle(void)
{
    forcedBattle = true;
    forcedBattleMonsterType = -1;
}

bool IsBattleForced(void)
{
    return forcedBattle;
}

static void InitializeBattleActorWrapper(BattleActorWrapper *actorWrapper, BattlerWrapper *battlerWrapper, uint16_t level, uint16_t currentHealth, uint16_t *skillCooldowns)
{
    actorWrapper->battlerWrapper = battlerWrapper;
    actorWrapper->actor.level = level;
    actorWrapper->actor.maxHealth = CombatantClass_GetHealth(BattlerWrapper_GetCombatantClass(battlerWrapper), level);
    actorWrapper->actor.skillQueued = false;
    actorWrapper->actor.activeSkill = INVALID_SKILL;
    actorWrapper->actor.counterSkill = INVALID_SKILL;
    actorWrapper->actor.currentTime = 0;
    for(int i = 0; i < MAX_STATUS_EFFECTS; ++i)
    {
        actorWrapper->actor.statusEffectDurations[i] = 0;
    }
    if(currentHealth == 0)
        actorWrapper->actor.currentHealth = actorWrapper->actor.maxHealth;
    else
        actorWrapper->actor.currentHealth = currentHealth;
    for(int i = 0; i < MAX_SKILLS_IN_LIST; ++i)
    {
        actorWrapper->actor.skillCooldowns[i] = skillCooldowns[i];
    }
    actorWrapper->actor.aiState.stage = 0;
    actorWrapper->actor.aiState.skillIndex = 0;
    actorWrapper->actor.timeInCombat = 0;
}

void Battle_InitializeNewMonster(uint16_t monsterIndex, bool fullHeal)
{
    Monster_UnloadCurrent();
    gBattleState.monster.battlerWrapper = NULL;
    Monster_LoadCurrent(monsterIndex);
    gBattleState.monster.battlerWrapper = BattlerWrapper_GetMonsterWrapper();
    currentMonsterIndex = monsterIndex;
    uint16_t skillCooldowns[MAX_SKILLS_IN_LIST] = {0};
    InitializeBattleActorWrapper(&gBattleState.monster, BattlerWrapper_GetMonsterWrapper(), Location_GetCurrentBaseLevel(), fullHeal ? 0 : gBattleState.monster.actor.currentHealth, skillCooldowns);
}

void BattleInit(void)
{
    DEBUG_LOG("BattleInit");
    Monster_UnloadCurrent();
    gBattleState.monster.battlerWrapper = NULL;
    bool loaded = false;
    
    if(forcedBattle && forcedBattleMonsterType > -1)
    {
        loaded = Monster_LoadCurrent(forcedBattleMonsterType);
        gBattleState.monster.battlerWrapper = BattlerWrapper_GetMonsterWrapper();
        gBattleState.player.battlerWrapper = BattlerWrapper_GetPlayerWrapper();
        currentMonsterIndex = forcedBattleMonsterType;
        forcedBattle = false;
    }
    else
    {
        forcedBattle = false;
        if(!Monster_Loaded())
        {
            currentMonsterIndex = Location_GetCurrentMonster();
            loaded = Monster_LoadCurrent(currentMonsterIndex);
        }
        if(loaded)
        {
            InitializeBattleActorWrapper(&gBattleState.player, BattlerWrapper_GetPlayerWrapper(), Character_GetLevel(), Character_GetHealth(), Character_GetCooldowns());
            uint16_t skillCooldowns[MAX_SKILLS_IN_LIST] = {0};
            InitializeBattleActorWrapper(&gBattleState.monster, BattlerWrapper_GetMonsterWrapper(), Location_GetCurrentBaseLevel(), 0, skillCooldowns);
        }
    }
    
    if(!loaded)
    {
        DEBUG_LOG("Aborting battle");
        // Monster prereqs failed
        GlobalState_Pop();
        return;
    }
    
    GRect playerHealthFrame = PLAYER_HEALTH_FRAME;
    GRect playerTimeFrame = PLAYER_TIME_FRAME;
    GRect monsterHealthFrame = MONSTER_HEALTH_FRAME;
    GRect monsterTimeFrame = MONSTER_TIME_FRAME;
    
    playerHealthBar = CreateProgressBar(&gBattleState.player.actor.currentHealth, &gBattleState.player.actor.maxHealth, FILL_UP, &playerHealthFrame, GColorBrightGreen, -1);
    monsterHealthBar = CreateProgressBar(&gBattleState.monster.actor.currentHealth, &gBattleState.monster.actor.maxHealth, FILL_DOWN, &monsterHealthFrame, GColorFolly, -1);
    playerTimeBar = CreateProgressBar(&gBattleState.player.actor.currentTime, &maxTimeCount, FILL_UP, &playerTimeFrame, GColorVeryLightBlue, -1);
    monsterTimeBar = CreateProgressBar(&gBattleState.monster.actor.currentTime, &maxTimeCount, FILL_DOWN, &monsterTimeFrame, GColorRichBrilliantLavender, -1);
    
    InitializeProgressBar(playerHealthBar, GetBaseWindow());
    InitializeProgressBar(playerTimeBar, GetBaseWindow());
    InitializeProgressBar(monsterHealthBar, GetBaseWindow());
    InitializeProgressBar(monsterTimeBar, GetBaseWindow());
    
#if defined(PBL_ROUND)
    HideDateLayer();
#endif
    
    // Force the main menu to the front
    InitializeMenuLayer(GetMainMenu(), GetBaseWindow());
    InitializeMenuLayer(GetSlaveMenu(), GetBaseWindow());
    
    // Force dialog layer to the top
    InitializeDialogLayer(GetBaseWindow());

    RegisterMenuState(GetMainMenu(), STATE_BATTLE);
    RegisterMenuState(GetSlaveMenu(), STATE_NONE);
    
    DEBUG_VERBOSE_LOG("Finished battle init");
    battleCleanExit = false;
}

static void DoSkill(Skill *skill, BattleActorWrapper *attacker, BattleActorWrapper *defender)
{
    gEffectDescription = ExecuteSkill(skill, attacker, defender);
    MarkProgressBarDirty(playerHealthBar);
    MarkProgressBarDirty(monsterHealthBar);
    DialogData *dialog = DialogData_Create("Skill Effect", gEffectDescription, false);
    Dialog_Trigger(dialog);
    attacker->actor.currentTime = 0;
    MarkProgressBarDirty(playerTimeBar);
    MarkProgressBarDirty(monsterTimeBar);
    attacker->actor.skillQueued = false;
}

static void UpdateActor(BattleActorWrapper *wrapper)
{
    uint16_t currentSpeed = 1;
    if(wrapper->actor.skillQueued)
    {
        currentSpeed = BattlerWrapper_GetSkillByIndex(wrapper->battlerWrapper, wrapper->actor.activeSkill)->speed;
    }
    else
    {
        currentSpeed = CombatantClass_GetSpeed(BattlerWrapper_GetCombatantClass(wrapper->battlerWrapper), wrapper->actor.level);
    }
    
    if(wrapper->actor.statusEffectDurations[STATUS_EFFECT_SLOW] > 0)
    {
        currentSpeed /= 2;
        if(currentSpeed <= 0)
            currentSpeed = 1;
    }
    
    if(wrapper->actor.statusEffectDurations[STATUS_EFFECT_HASTE] > 0)
    {
        currentSpeed *= 2;
    }
    
    wrapper->actor.currentTime += currentSpeed;
    wrapper->actor.timeInCombat++;
}

static void UpdateStatusEffects(BattleActorWrapper *wrapper)
{
    for(int i = 0; i < MAX_STATUS_EFFECTS; ++i)
    {
        if(wrapper->actor.statusEffectDurations[i] > 0)
        {
            if(i == STATUS_EFFECT_POISON)
            {
                int damage = wrapper->actor.maxHealth * (POISON_DAMAGE_PERCENT) / 100;
                if(damage < 1)
                    damage = 1;
                DealDamage(damage, &wrapper->actor);
            }
            wrapper->actor.statusEffectDurations[i]--;
        }
    }
}

void UpdateBattle(void *unused)
{
    if(battleSaveTickDelay > 0)
    {
        --battleSaveTickDelay;
    }
    
    if(gPlayerTurn)
        return;
    
    if(gBattleState.player.actor.currentHealth <= 0 || gBattleState.monster.actor.currentHealth <= 0)
    {
        CloseBattleWindow();
        return;
    }
    
    if(BattleEvent_TriggerAutomaticBattleEvents())
    {
        return;
    }
    
    bool playerAhead = gBattleState.player.actor.currentTime >= gBattleState.monster.actor.currentTime;
    bool actionPerformed = false;
    
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
                if(gBattleState.player.actor.statusEffectDurations[STATUS_EFFECT_STUN] == 0)
                {
                    UpdateSkillCooldowns(gBattleState.player.actor.skillCooldowns);
                    gPlayerTurn = true;
                    gPlayerActed = false;
                    RegisterMenuState(GetMainMenu(), STATE_BATTLE);
                    RegisterMenuState(GetSlaveMenu(), STATE_NONE);
                    Menu_ResetSelection(GetMainMenu());
                    DialogData *dialog = DialogData_Create("Player's Turn", "Choose an action.", false);
                    Dialog_Trigger(dialog);
                    QueueMenu(GetMainMenu());
                }
                else
                {
                    DialogData *dialog = DialogData_Create("Player's Turn", "You are stunned and cannot act.", false);
                    Dialog_Trigger(dialog);
                    gBattleState.player.actor.currentTime = 0;
                    MarkProgressBarDirty(playerTimeBar);
                    MarkProgressBarDirty(monsterTimeBar);
                }
                UpdateStatusEffects(&gBattleState.player);
            }
            actionPerformed = true;
        }
    }
    else
    {
        if(gBattleState.monster.actor.currentTime >= maxTimeCount)
        {
            if(gBattleState.monster.actor.skillQueued)
            {
                bool act = true;
                Skill *skill = BattlerWrapper_GetSkillByIndex(gBattleState.monster.battlerWrapper, gBattleState.monster.actor.activeSkill);
                if(gBattleState.monster.actor.statusEffectDurations[STATUS_EFFECT_SILENCE] > 0 ||
                   gBattleState.monster.actor.statusEffectDurations[STATUS_EFFECT_PASSIFY] > 0)
                {
                    if((skill->damageType & DAMAGE_TYPE_MAGIC) && gBattleState.monster.actor.statusEffectDurations[STATUS_EFFECT_SILENCE])
                        act = false;
                    if((skill->damageType & DAMAGE_TYPE_PHYSICAL) && gBattleState.monster.actor.statusEffectDurations[STATUS_EFFECT_PASSIFY])
                        act = false;
                }
                if(act)
                    DoSkill(skill, &gBattleState.monster, &gBattleState.player);
            }
            else
            {
                UpdateSkillCooldowns(gBattleState.monster.actor.skillCooldowns);
                gBattleState.monster.actor.currentTime = 0;
                bool act = true;
                if(gBattleState.monster.actor.statusEffectDurations[STATUS_EFFECT_STUN] > 0)
                    act = false;
                if(gBattleState.monster.actor.statusEffectDurations[STATUS_EFFECT_SILENCE] > 0 ||
                   gBattleState.monster.actor.statusEffectDurations[STATUS_EFFECT_PASSIFY] > 0)
                {
                    Skill *skill = BattlerWrapper_GetSkillByIndex(gBattleState.monster.battlerWrapper, 0);
                    if((skill->damageType & DAMAGE_TYPE_MAGIC) && gBattleState.monster.actor.statusEffectDurations[STATUS_EFFECT_SILENCE])
                        act = false;
                    if((skill->damageType & DAMAGE_TYPE_PHYSICAL) && gBattleState.monster.actor.statusEffectDurations[STATUS_EFFECT_PASSIFY])
                        act = false;
                }
                if(act)
                {
                    ExecuteAI(&gBattleState.monster, &gBattleState.player);
                    Skill *skill = BattlerWrapper_GetSkillByIndex(gBattleState.monster.battlerWrapper, gBattleState.monster.actor.activeSkill);
                    if(skill)
                    {
                        char text[MAX_DIALOG_LENGTH] = {0};
                        snprintf(text, MAX_DIALOG_LENGTH, "%s prepares %s", Monster_GetCurrentName(), skill->name);
                        DialogData *dialog = DialogData_Create("", text, false);
                        Dialog_Trigger(dialog);
                    }
                }
                UpdateStatusEffects(&gBattleState.monster);
            }
            actionPerformed = true;
        }
    }

    if(!actionPerformed)
    {
        UpdateActor(&gBattleState.player);
        UpdateActor(&gBattleState.monster);

        MarkProgressBarDirty(playerTimeBar);
        MarkProgressBarDirty(monsterTimeBar);
    }
}

void BattleScreenPush(void *data)
{
    BattleInit();
}

void BattleScreenPop(void *data)
{
    DEBUG_LOG("BattleScreenPop");
    RemoveProgressBar(playerHealthBar);
    RemoveProgressBar(playerTimeBar);
    RemoveProgressBar(monsterHealthBar);
    RemoveProgressBar(monsterTimeBar);
    FreeProgressBar(playerHealthBar);
    playerHealthBar = NULL;
    FreeProgressBar(playerTimeBar);
    playerTimeBar = NULL;
    FreeProgressBar(monsterHealthBar);
    monsterHealthBar = NULL;
    FreeProgressBar(monsterTimeBar);
    monsterTimeBar = NULL;
    Monster_UnloadCurrent();
}

void TriggerBattleScreen(void)
{
    if(Location_CurrentLocationHasMonster())
        GlobalState_Push(STATE_BATTLE, SECOND_UNIT, NULL);
}
