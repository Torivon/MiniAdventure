#include "pebble.h"

#include "Adventure.h"
#include "Battle.h"
#include "Character.h"
#include "DescriptionFrame.h"
#include "Events.h"
#include "GlobalState.h"
#include "Items.h"
#include "Logging.h"
#include "MainImage.h"
#include "MainMenu.h"
#include "Menu.h"
#include "NewBattle.h"
#include "NewMenu.h"
#include "Persistence.h"
#include "OptionsMenu.h"
#include "Shop.h"
#include "Story.h"
#include "UILayers.h"
#include "Utils.h"
#include "WorkerControl.h"

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
	if(!LoadPersistedData())
		ResetGame();
}

void ResetGame(void)
{
	INFO_LOG("Resetting game.");
#if ENABLE_SHOPS
	ResetStatPointsPurchased();
#endif
	InitializeCharacter();
#if ENABLE_ITEMS
	ClearInventory();
#endif
	InitializeCurrentStory();
	
	SavePersistedData();
}

static uint16_t AdventureMenuCount(void)
{
	if(IsCurrentLocationPath())
		return 0;
	
	DEBUG_LOG("AdjacentLocationCount %d", GetCurrentLocationAdjacentLocations());
	return GetCurrentLocationAdjacentLocations();
}

static const char *AdventureMenuNameCallback(int row)
{
	if(IsCurrentLocationPath())
		return NULL;

	DEBUG_LOG("AdjacentLocationName %s", GetAdjacentLocationName(row));
	return GetAdjacentLocationName(row);	
}

static void AdventureMenuSelectCallback(int row)
{
	DEBUG_LOG("Trying to follow path");
	if(!IsCurrentLocationPath())
	{
		newLocation = row;
	}	
}

void RefreshAdventure(void)
{
	if(!gUpdateAdventure)
		return;
	
	DEBUG_VERBOSE_LOG("Refreshing adventure window. %s", GetCurrentLocationName());
	updateDelay = 1;
	LoadLocationImage();
	ReloadMenu();
	SetDescription(GetCurrentLocationName()); //Add floor back in somehow
}

void LoadLocationImage(void)
{
	adventureImageId = GetCurrentBackgroundImage();
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

void UpdateAdventure(void)
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

void AdventureScreenPush(void)
{
	CurrentStoryStateNeedsSaving();
	InitializeGameData();	
}

void AdventureScreenAppear(void)
{
	gUpdateAdventure = true;
	RegisterMenuCellCallbacks(AdventureMenuCount, AdventureMenuNameCallback, AdventureMenuNameCallback, AdventureMenuSelectCallback);
	if(newLocation > -1)
	{
		TravelToAdjacentLocationByIndex(newLocation);
	}
	newLocation = -1;
	RefreshAdventure();
}

void AdventureScreenDisappear(void)
{
	gUpdateAdventure = false;
}

void AdventureScreenPop(void)
{
	SavePersistedData();
	ClearCurrentStory();
}

void TriggerAdventureScreen(void)
{
	PushGlobalState(ADVENTURE, MINUTE_UNIT, UpdateAdventure, AdventureScreenPush, AdventureScreenAppear, AdventureScreenDisappear, AdventureScreenPop);
}
