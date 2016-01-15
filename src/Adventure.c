#include "pebble.h"

#include "Adventure.h"
#include "BinaryResourceLoading.h"
#include "Character.h"
#include "Clock.h"
#include "DescriptionFrame.h"
#include "DialogFrame.h"
#include "EngineInfo.h"
#include "ExtraMenu.h"
#include "GlobalState.h"
#include "Logging.h"
#include "MainImage.h"
#include "Battle.h"
#include "Menu.h"
#include "BaseWindow.h"
#include "Persistence.h"
#include "ProgressBar.h"
#include "OptionsMenu.h"
#include "ResourceStory.h"
#include "Utils.h"
#include "WorkerControl.h"

#if defined(PBL_ROUND)
#define LOCATION_PROGRESS_FRAME {.origin = {.x = 59, .y = 67}, .size = {.w = 16, .h = 46}}
#else
#define LOCATION_PROGRESS_FRAME {.origin = {.x = 133, .y = 48}, .size = {.w = 16, .h = 84}}
#endif

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
    ResourceStory_InitializeCurrent();
    Character_Initialize();
    
    SaveStoryPersistedData();
    playOpeningDialog = true;
    skipFinalSave = false;
    dead = false;
    Battle_SetCleanExit();
}

uint16_t Adventure_MenuSectionCount(void)
{
    return 3 + ExtraMenu_GetSectionCount();
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
            return ExtraMenu_GetSectionName();
    }
    return "None";
}

uint16_t Adventure_MenuCellCount(uint16_t sectionIndex)
{
    switch(sectionIndex)
    {
        case 0:
        {
            if(ResourceStory_CurrentLocationIsPath())
                return 0;
            
            return ResourceStory_GetCurrentLocalEvents();
            break;
        }
        case 1:
        {
            if(ResourceStory_CurrentLocationIsPath())
                return 0;
            
            return ResourceStory_GetCurrentAdjacentLocations();
            break;
        }
        case 2:
        {
            return 4;
        }
        case 3:
        {
            return ExtraMenu_GetCellCount();
        }
    }
    return 0;
}

const char *Adventure_MenuCellName(MenuIndex *index)
{
    switch(index->section)
    {
        case 0:
            return ResourceStory_GetLocalEventName(index->row);
        case 1:
            return ResourceStory_GetAdjacentLocationName(index->row);
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
            return ExtraMenu_GetCellName(index->row);
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
                    DialogData *dialog = calloc(sizeof(DialogData), 1);
                    ResourceLoadStruct(EngineInfo_GetResHandle(), EngineInfo_GetInfo()->resetPromptDialog, (uint8_t*)dialog, sizeof(DialogData), "DialogData");
                    dialog->allowCancel = true;
                    QueueDialog(dialog);
                    GlobalState_Queue(STATE_RESET_GAME, 0, NULL);
                    break;
                }
            }
            break;
        }
        case 3:
        {
            ExtraMenu_SelectAction(index->row);
            break;
        }
    }
}

void UpdateLocationProgress(void)
{
    if(ResourceStory_CurrentLocationIsPath())
    {
        ShowProgressBar(locationProgress);
        currentProgress = ResourceStory_GetTimeOnPath();
        maxProgress = ResourceStory_GetCurrentLocationLength();
        MarkProgressBarDirty(locationProgress);
    }
    else
    {
        HideProgressBar(locationProgress);
    }
}

void RefreshAdventure(void)
{
    if(!gUpdateAdventure)
        return;
    
    updateDelay = 1;
    LoadLocationImage();
    ReloadMenu(GetMainMenu());
    SetDescription(ResourceStory_GetCurrentLocationName()); //Add floor back in somehow
    UpdateLocationProgress();
}

void LoadLocationImage(void)
{
    adventureImageId = ResourceStory_GetCurrentLocationBackgroundImageId();
    SetBackgroundImage(adventureImageId);
    SetMainImageVisibility(true, false, true);
}

bool ComputeRandomEvent(void)
{
    int result = Random(100) + 1;
    int chanceOfEvent = ResourceStory_GetCurrentLocationEncounterChance();
    
    if(result > chanceOfEvent)
        return false;
    
    if(GetVibration())
        vibes_short_pulse();
    
    TriggerBattleScreen();
    
    return true;
}

static void StoryUpdateResponse(ResourceStoryUpdateReturnType returnVal, bool vibration)
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
                break;
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
            ResourceStory_TriggerDialog(ResourceStory_GetWinDialogIndex());
            ClearCurrentStoryPersistedData();
            skipFinalSave = true;
            GlobalState_QueueStatePop();
            break;
        }
        default:
        {
        }
    }
}

void UpdateAdventure(void *data)
{
    ResourceStoryUpdateReturnType returnVal;
    
    if(updateDelay)
    {
        --updateDelay;
        return;
    }

    returnVal = ResourceStory_UpdateCurrentLocation();
    
    StoryUpdateResponse(returnVal, true);
}

void AdventureScreenPush(void *data)
{
    GRect locationProgressFrame = LOCATION_PROGRESS_FRAME;
    locationProgress = CreateProgressBar(&currentProgress, &maxProgress, FILL_UP, &locationProgressFrame, GColorYellow, -1);
    InitializeProgressBar(locationProgress, GetBaseWindow());
    
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
        ResourceEvent_Trigger(newEvent);
    }
    newEvent = -1;
    ResourceStoryUpdateReturnType returnVal = STORYUPDATE_FULLREFRESH;
    if(newLocation > -1)
    {
        int temp = newLocation;
        newLocation = -1;
        returnVal = ResourceStory_MoveToLocation(temp);
    }
    StoryUpdateResponse(returnVal, false);

    if(!dead && Character_GetHealth() <= 0)
    {
        DialogData *dialog = calloc(sizeof(DialogData), 1);
        ResourceLoadStruct(EngineInfo_GetResHandle(), EngineInfo_GetInfo()->gameOverDialog, (uint8_t*)dialog, sizeof(DialogData), "DialogData");
        TriggerDialog(dialog);
        ClearCurrentStoryPersistedData();
        skipFinalSave = true;
        GlobalState_QueueStatePop();
        dead = true;
        return;
    }

    if(playOpeningDialog)
    {
        ResourceStory_TriggerDialog(ResourceStory_GetOpeningDialogIndex());
        playOpeningDialog = false;
    }
}

void AdventureScreenDisappear(void *data)
{
    gUpdateAdventure = false;
    HideProgressBar(locationProgress);
}

void AdventureScreenPop(void *data)
{
    if(!skipFinalSave)
        SaveStoryPersistedData();
    ResourceStory_ClearCurrentStory();
    RemoveProgressBar(locationProgress);
    FreeProgressBar(locationProgress);
}

void TriggerAdventureScreen(void)
{
    GlobalState_Push(STATE_ADVENTURE, MINUTE_UNIT, NULL);
}

void QueueAdventureScreen(void)
{
    GlobalState_Queue(STATE_ADVENTURE, MINUTE_UNIT, NULL);
}
