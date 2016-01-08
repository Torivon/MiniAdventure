#include "pebble.h"

#include "Adventure.h"
#include "Character.h"
#include "DescriptionFrame.h"
#include "DialogFrame.h"
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
static GRect locationProgressFrame = {.origin = {.x = 59, .y = 67}, .size = {.w = 16, .h = 46}};
#else
static GRect locationProgressFrame = {.origin = {.x = 133, .y = 48}, .size = {.w = 16, .h = 84}};
#endif

static ProgressBar *locationProgress;
static uint16_t currentProgress = 0;
static uint16_t maxProgress = 1;

bool gUpdateAdventure = false;

static int updateDelay = 0;

static int adventureImageId = RESOURCE_ID_IMAGE_DUNGEONRIGHT;

static int newLocation = -1;

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
}

static uint16_t AdventureMenuSectionCount(void)
{
    return 2 + ExtraMenu_GetSectionCount();
}

static const char *AdventureMenuSectionName(uint16_t sectionIndex)
{
    switch(sectionIndex)
    {
        case 0:
            return "Locations";
        case 1:
            return "Story";
        case 2:
            return ExtraMenu_GetSectionName();
    }
    return "None";
}

static uint16_t AdventureMenuCount(uint16_t sectionIndex)
{
    switch(sectionIndex)
    {
        case 0:
        {
            if(ResourceStory_CurrentLocationIsPath())
                return 0;
            
            return ResourceStory_GetCurrentAdjacentLocations();
            break;
        }
        case 1:
        {
            return 1;
        }
        case 2:
        {
            return ExtraMenu_GetCellCount();
        }
    }
    return 0;
}

static const char *AdventureMenuNameCallback(MenuIndex *index)
{
    switch(index->section)
    {
        case 0:
            return ResourceStory_GetAdjacentLocationName(index->row);
        case 1:
            return "Reset";
        case 2:
            return ExtraMenu_GetCellName(index->row);
    }
    return "None";
}

static DialogData resetPrompt =
{
    .text = "Are you sure you want to reset the game?",
    .allowCancel = true
};

static void ResetGamePush(void *data)
{
    GlobalState_Pop();
}

static void ResetGamePop(void *data)
{
    ResetGame();
}

static void AdventureMenuSelectCallback(MenuIndex *index)
{
    switch(index->section)
    {
        case 0:
        {
            newLocation = index->row;
            break;
        }
        case 1:
        {
            QueueDialog(&resetPrompt);
            GlobalState_Queue(STATE_RESET_GAME, 0, NULL, ResetGamePush, NULL, NULL, ResetGamePop, NULL);
            break;
        }
        case 2:
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

void UpdateAdventure(void *data)
{
    ResourceStoryUpdateReturnType returnVal;
    
    if(IsBattleForced())
    {
        INFO_LOG("Triggering forced battle.");
        TriggerBattleScreen();
        return;
    }
    
    if(updateDelay)
    {
        --updateDelay;
        return;
    }

    returnVal = ResourceStory_UpdateCurrentLocation();
    
    switch(returnVal)
    {
        case STORYUPDATE_COMPUTERANDOM:
        {
            if(ComputeRandomEvent())
                break;
            LoadLocationImage();
            UpdateLocationProgress();
            break;
        }
        case STORYUPDATE_DONOTHING:
        {
            break;
        }
        case STORYUPDATE_FULLREFRESH:
        {
            if(GetVibration())
                vibes_short_pulse();
            
            RefreshAdventure();
            Menu_ResetSelection(GetMainMenu());
            break;
        }
        case STORYUPDATE_WIN:
        {
            ResetGame();
            GlobalState_Pop();
            break;
        }
    }
}

void AdventureScreenPush(void *data)
{
    InitializeGameData();
    locationProgress = CreateProgressBar(&currentProgress, &maxProgress, FILL_UP, locationProgressFrame, GColorYellow, -1);
    InitializeProgressBar(locationProgress, GetBaseWindow());
    UpdateLocationProgress();
    
    // Force the main menu to the front
    InitializeMenuLayer(GetMainMenu(), GetBaseWindow());
    InitializeMenuLayer(GetSlaveMenu(), GetBaseWindow());
    
    // Force dialog layer to the top
    InitializeDialogLayer(GetBaseWindow());
}

void AdventureScreenAppear(void *data)
{
    gUpdateAdventure = true;
    if(Character_GetHealth() <= 0)
    {
        ResetGame();
    }
    UpdateLocationProgress();
    RegisterMenuCellCallbacks(GetMainMenu(), AdventureMenuSectionName, AdventureMenuSectionCount, AdventureMenuCount, AdventureMenuNameCallback, AdventureMenuNameCallback, AdventureMenuSelectCallback);
    ResourceStoryUpdateReturnType returnVal = STORYUPDATE_FULLREFRESH;
    if(newLocation > -1)
    {
        returnVal = ResourceStory_MoveToLocation(newLocation);
    }
    newLocation = -1;
    if(returnVal == STORYUPDATE_WIN)
    {
        ResetGame();
        GlobalState_Pop();
        return;
    }
    
    if(returnVal == STORYUPDATE_FULLREFRESH)
        RefreshAdventure();
    
    if(IsBattleForced())
    {
        TriggerBattleScreen();
    }
}

void AdventureScreenDisappear(void *data)
{
    gUpdateAdventure = false;
    HideProgressBar(locationProgress);
}

void AdventureScreenPop(void *data)
{
    SaveStoryPersistedData();
    ResourceStory_ClearCurrentStory();
    RemoveProgressBar(locationProgress);
    FreeProgressBar(locationProgress);
}

void QueueAdventureScreen(void)
{
    GlobalState_Queue(STATE_ADVENTURE, MINUTE_UNIT, UpdateAdventure, AdventureScreenPush, AdventureScreenAppear, AdventureScreenDisappear, AdventureScreenPop, NULL);
}
