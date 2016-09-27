#include <pebble.h>

#include "Adventure.h"
#include "BaseWindow.h"
#include "BinaryResourceLoading.h"
#include "Character.h"
#include "Clock.h"
#include "DescriptionFrame.h"
#include "DialogFrame.h"
#include "EngineInfo.h"
#include "EngineMenu.h"
#include "Events.h"
#include "GlobalState.h"
#include "Location.h"
#include "Logging.h"
#include "MainImage.h"
#include "Battle.h"
#include "Menu.h"
#include "OptionsMenu.h"
#include "Persistence.h"
#include "ProgressBar.h"
#include "Story.h"
#include "Utils.h"
#include "WorkerControl.h"

static ProgressBar *locationProgress = NULL;
static uint16_t currentProgress = 0;
static uint16_t maxProgress = 1;

bool gUpdateAdventure = false;

static int updateDelay = 0;

static int adventureImageId = -1;

static int newEvent = -1;
static int newLocation = -1;

static bool firstLaunch = true;
static bool playOpeningDialog = false;
static bool skipFinalSave = false;
static bool dead = false;

void SetUpdateDelay(void)
{
    updateDelay = 1;
}

void LoadLocationImage(void);

void InitializeGameData(void)
{
    if(!LoadStoryPersistedData())
        ResetGame();
}

void ResetGame(void)
{
    INFO_LOG("Resetting game.");
    Story_InitializeCurrent();
    Character_Initialize();
    
    SaveStoryPersistedData();
    playOpeningDialog = true;
    skipFinalSave = false;
    dead = false;
    Battle_SetCleanExit();
}

uint16_t Adventure_MenuSectionCount(void)
{
    return 3 + EngineMenu_GetSectionCount();
}

const char *Adventure_MenuSectionName(uint16_t sectionIndex)
{
    switch(sectionIndex)
    {
        case 0:
            return "Events";
        case 1:
            return "Locations";
        case 2:
            return "Story";
        case 3:
            return EngineMenu_GetSectionName();
    }
    return "None";
}

uint16_t Adventure_MenuCellCount(uint16_t sectionIndex)
{
    switch(sectionIndex)
    {
        case 0:
        {
            if(Location_CurrentLocationIsPath())
                return 0;
            
            return Event_GetCurrentLocalEvents();
            break;
        }
        case 1:
        {
            if(Location_CurrentLocationIsPath())
                return 0;
            
            return Location_GetCurrentAdjacentLocations();
            break;
        }
        case 2:
        {
            return 6;
        }
        case 3:
        {
            return EngineMenu_GetCellCount();
        }
    }
    return 0;
}

const char *Adventure_MenuCellName(MenuIndex *index)
{
    switch(index->section)
    {
        case 0:
            return Event_GetLocalEventName(index->row);
        case 1:
            return Location_GetAdjacentLocationName(index->row);
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
                    return "Key Items";
                case 4:
                    return "Credits";
                case 5:
                    return "Reset";
            }
            break;
        }
        case 3:
            return EngineMenu_GetCellName(index->row);
    }
    return "None";
}

const char *Adventure_MenuCellDescription(MenuIndex *index)
{
    switch(index->section)
    {
        case 0:
            return Event_GetLocalEventDescription(index->row);
        case 1:
            return Location_GetAdjacentLocationDescription(index->row);
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
                    return "Key Items";
                case 4:
                    return "Credits";
                case 5:
                    return "Reset";
            }
            break;
        }
        case 3:
            return EngineMenu_GetCellName(index->row);
    }
    return "None";
}

void ResetGamePush(void *data)
{
    GlobalState_Pop();
}

void ResetGamePop(void *data)
{
    ResetGame();
}

void Adventure_MenuSelect(MenuIndex *index)
{
    switch(index->section)
    {
        case 0:
        {
            newEvent = index->row;
            break;
        }
        case 1:
        {
            newLocation = index->row;
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
                    Character_ShowKeyItems();
                    break;
                }
                case 4:
                {
                    Story_QueueCreditsDialog();
                    break;
                }
                case 5:
                {
                    Dialog_QueueFromResource(EngineInfo_GetResHandle(), EngineInfo_GetInfo()->resetPromptDialog);
                    GlobalState_Queue(STATE_RESET_GAME, 0, NULL);
                    break;
                }
            }
            break;
        }
        case 3:
        {
            EngineMenu_SelectAction(index->row);
            break;
        }
    }
}

void UpdateLocationProgress(void)
{
    if(Location_CurrentLocationIsPath())
    {
        ProgressBar_Show(locationProgress);
        currentProgress = Story_GetTimeOnPath();
        maxProgress = Location_GetCurrentLength();
        ProgressBar_MarkDirty(locationProgress);
    }
    else
    {
        ProgressBar_Hide(locationProgress);
    }
}

void RefreshAdventure(void)
{
    if(!gUpdateAdventure)
        return;
    
    updateDelay = 1;
    LoadLocationImage();
    ReloadMenu(GetMainMenu());
    SetDescription(Location_GetCurrentName());
    UpdateLocationProgress();
}

void LoadLocationImage(void)
{
    adventureImageId = Location_GetCurrentBackgroundImageId();
    SetBackgroundImage(adventureImageId);
    SetMainImageVisibility(true, false, true);
}

bool ComputeRandomEvent(void)
{
    int result = Random(100) + 1;
    int chanceOfEvent = Location_GetCurrentEncounterChance();
    
    if(result > chanceOfEvent)
        return false;
    
    return true;
}

static void StoryUpdateResponse(StoryUpdateReturnType returnVal, bool vibration)
{
    switch(returnVal)
    {
        case STORYUPDATE_TRIGGER_BATTLE:
        {
            TriggerBattleScreen();
            break;
        }
        case STORYUPDATE_COMPUTERANDOM:
        {
            if(ComputeRandomEvent())
            {
                if(GetVibration())
                    vibes_short_pulse();
                TriggerBattleScreen();
                break;
            }
            LoadLocationImage();
            UpdateLocationProgress();
            break;
        }
        case STORYUPDATE_FULLREFRESH:
        {
            if(vibration && GetVibration())
                vibes_short_pulse();
            
            RefreshAdventure();
            Menu_ResetSelection(GetMainMenu());
            break;
        }
        case STORYUPDATE_WIN:
        {
            RefreshAdventure();
            Menu_ResetSelection(GetMainMenu());
            Story_TriggerWinDialog();
            ClearCurrentStoryPersistedData();
            skipFinalSave = true;
            GlobalState_QueueStatePop();
            break;
        }
        case STORYUPDATE_SKIP_ENCOUNTER_WITH_XP:
        {
            if(ComputeRandomEvent())
                Character_GrantXP(Location_GetCurrentBaseLevel());
            // Intentionally fall through
        }
        case STORYUPDATE_SKIP_ENCOUNTER_NO_XP:
        {
            LoadLocationImage();
            UpdateLocationProgress();
            break;
        }
        default:
        {
        }
    }
}

void UpdateAdventure(void *data)
{
    StoryUpdateReturnType returnVal;
    
    if(updateDelay)
    {
        --updateDelay;
        return;
    }

    returnVal = Story_UpdateCurrentLocation();
    
    StoryUpdateResponse(returnVal, true);
}

void AdventureScreenPush(void *data)
{
    GRect locationProgressFrame = LOCATION_PROGRESS_FRAME;
    locationProgress = ProgressBar_Create(&currentProgress, &maxProgress, FILL_UP, &locationProgressFrame, GColorYellow, -1);
    ProgressBar_Initialize(locationProgress, window_get_root_layer(GetBaseWindow()));
    
    // Force the main menu to the front
    InitializeMenuLayer(GetMainMenu(), GetBaseWindow());
    InitializeMenuLayer(GetSlaveMenu(), GetBaseWindow());
    // Force dialog layer to the top
    InitializeDialogLayer(GetBaseWindow());

    InitializeGameData();
    UpdateLocationProgress();
    dead = false;
}

void AdventureScreenAppear(void *data)
{
    gUpdateAdventure = true;

    UpdateLocationProgress();
    RegisterMenuState(GetMainMenu(), STATE_ADVENTURE);
    RegisterMenuState(GetSlaveMenu(), STATE_NONE);
    ShowDateLayer();
    
    if(IsBattleForced())
    {
        INFO_LOG("Triggering forced battle.");
        TriggerBattleScreen();
        firstLaunch = false;
        return;
    }
    
    if(launch_reason() == APP_LAUNCH_WORKER && firstLaunch)
    {
        updateDelay = 0;
        UpdateAdventure(NULL);
        updateDelay = 1;
    }
    firstLaunch = false;

    if(newEvent > -1)
    {
        int eventToTrigger = newEvent;
        newEvent = -1;
        Event_Trigger(eventToTrigger);
    }
    StoryUpdateReturnType returnVal = STORYUPDATE_FULLREFRESH;
    if(newLocation > -1)
    {
        int temp = newLocation;
        newLocation = -1;
        returnVal = Story_MoveToAdjacentLocation(temp);
    }
    StoryUpdateResponse(returnVal, false);

    if(!dead && Character_GetHealth() <= 0)
    {
        Dialog_TriggerFromResource(EngineInfo_GetResHandle(), EngineInfo_GetInfo()->gameOverDialog);
        if(Story_GetCurrentStoryAllowRespawn())
        {
            Story_MoveToRespawnPoint();
        }
        else
        {
            ClearCurrentStoryPersistedData();
            skipFinalSave = true;
            GlobalState_QueueStatePop();
            dead = true;
        }
        return;
    }

    if(playOpeningDialog)
    {
        Story_TriggerOpeningDialog();
        playOpeningDialog = false;
    }
}

void AdventureScreenDisappear(void *data)
{
    gUpdateAdventure = false;
    ProgressBar_Hide(locationProgress);
}

void AdventureScreenPop(void *data)
{
    if(!skipFinalSave)
        SaveStoryPersistedData();
    Story_ClearCurrentStory();
    ProgressBar_Remove(locationProgress);
    ProgressBar_Free(locationProgress);
}

void Adventure_Trigger(void)
{
    GlobalState_Push(STATE_ADVENTURE, MINUTE_UNIT, NULL);
}

void Adventure_Queue(void)
{
    GlobalState_Queue(STATE_ADVENTURE, MINUTE_UNIT, NULL);
}
