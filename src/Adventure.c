#include "pebble.h"

#include "Adventure.h"
#include "Character.h"
#include "DescriptionFrame.h"
#include "Events.h"
#include "GlobalState.h"
#include "Logging.h"
#include "MainImage.h"
#include "NewBattle.h"
#include "NewMenu.h"
#include "NewBaseWindow.h"
#include "Persistence.h"
#include "ProgressBar.h"
#include "OptionsMenu.h"
#include "ResourceStory.h"
#include "Story.h"
#include "Utils.h"
#include "WorkerControl.h"

#if defined(PBL_ROUND)
static GRect locationProgressFrame = {.origin = {.x = 59, .y = 67}, .size = {.w = 16, .h = 46}};
#else
static GRect locationProgressFrame = {.origin = {.x = 133, .y = 48}, .size = {.w = 16, .h = 84}};
#endif

static ProgressBar *locationProgress;
static int currentProgress = 0;
static int maxProgress = 1;

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
    // TODO:
    ResetGame();
    
/*    if(!LoadPersistedData())
        ResetGame();*/
}

void ResetGame(void)
{
    INFO_LOG("Resetting game.");
    Character_Initialize();
    ResourceStory_InitializeCurrent();
    
    SavePersistedData();
}

static uint16_t AdventureMenuCount(void)
{
    return ResourceStory_GetCurrentAdjacentLocations();
}

static const char *AdventureMenuNameCallback(int row)
{
    return "Location";
}

static void AdventureMenuSelectCallback(int row)
{
    ResourceStory_MoveToLocation(row);
    LoadLocationImage();
}

void UpdateLocationProgress(void)
{
    if(IsCurrentLocationFixed())
    {
        HideProgressBar(locationProgress);
    }
    else
    {
        ShowProgressBar(locationProgress);
        currentProgress = GetCurrentDuration();
        maxProgress = GetCurrentLocationLength();
        MarkProgressBarDirty(locationProgress);
    }
}

void RefreshAdventure(void)
{
    if(!gUpdateAdventure)
        return;
    
    DEBUG_VERBOSE_LOG("Refreshing adventure window. %s", GetCurrentLocationName());
    updateDelay = 1;
    LoadLocationImage();
    ReloadMenu(GetMainMenu());
    SetDescription(ResourceStory_GetCurrentLocationName()); //Add floor back in somehow
    UpdateLocationProgress();
}

void LoadLocationImage(void)
{
    adventureImageId = ResourceStory_GetCurrentLocationBackgroundImageId();
    DEBUG_VERBOSE_LOG("Loading image: %d", adventureImageId);
    SetBackgroundImage(adventureImageId);
    SetMainImageVisibility(true, false, true);
}

typedef void (*ShowWindowFunction)(void);

typedef struct
{
    ShowWindowFunction windowFunction;
    int weight;
} RandomTableEntry;

// These should add up to 100
RandomTableEntry entries[] =
{
    {TriggerBattleScreen, 100},
};

bool ComputeRandomEvent(void)
{
    int result = Random(100) + 1;
    int i = 0;
    int acc = 0;
    int chanceOfEvent = GetCurrentLocationEncounterChance();
    
    if(result > chanceOfEvent)
        return false;
    
    result = Random(100) + 1;
    
    do
    {
        acc += entries[i].weight;
        if(acc >= result)
        {
            if(GetVibration())
                vibes_short_pulse();
            if(entries[i].windowFunction)
                entries[i].windowFunction();
            break;
        }
        ++i;
    } while (i < 4);
    return true;
}

void UpdateAdventure(void *data)
{
    LocationUpdateReturnType returnVal;
    
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
    
    returnVal = UpdateCurrentLocation();
    
    switch(returnVal)
    {
        case LOCATIONUPDATE_COMPUTERANDOM:
            ComputeRandomEvent();
            LoadLocationImage();
            UpdateLocationProgress();
            break;
        case LOCATIONUPDATE_DONOTHING:
            break;
        case LOCATIONUPDATE_FULLREFRESH:
            if(GetVibration())
                vibes_short_pulse();
            
            RefreshAdventure();
            break;
    }
}

void AdventureScreenPush(void *data)
{
    CurrentStoryStateNeedsSaving();
    InitializeGameData();
    locationProgress = CreateProgressBar(&currentProgress, &maxProgress, FILL_UP, locationProgressFrame, GColorYellow, -1);
    InitializeProgressBar(locationProgress, GetBaseWindow());
    UpdateLocationProgress();
    
    // Force the main menu to the front
    InitializeNewMenuLayer(GetMainMenu(), GetBaseWindow());
    InitializeNewMenuLayer(GetSlaveMenu(), GetBaseWindow());
}

void AdventureScreenAppear(void *data)
{
    gUpdateAdventure = true;
    RegisterMenuCellCallbacks(GetMainMenu(), AdventureMenuCount, AdventureMenuNameCallback, AdventureMenuNameCallback, AdventureMenuSelectCallback);
    if(newLocation > -1)
    {
        TravelToAdjacentLocationByIndex(newLocation);
    }
    newLocation = -1;
    RefreshAdventure();
}

void AdventureScreenDisappear(void *data)
{
    gUpdateAdventure = false;
}

void AdventureScreenPop(void *data)
{
    SavePersistedData();
    ClearCurrentStory();
    ResourceStory_FreeCurrent();
    RemoveProgressBar(locationProgress);
    FreeProgressBar(locationProgress);
}

void TriggerAdventureScreen(void)
{
    PushGlobalState(STATE_ADVENTURE, MINUTE_UNIT, UpdateAdventure, AdventureScreenPush, AdventureScreenAppear, AdventureScreenDisappear, AdventureScreenPop, NULL);
}

void QueueAdventureScreen(void)
{
    QueueGlobalState(STATE_ADVENTURE, MINUTE_UNIT, UpdateAdventure, AdventureScreenPush, AdventureScreenAppear, AdventureScreenDisappear, AdventureScreenPop, NULL);
}
