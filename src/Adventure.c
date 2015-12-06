#include "pebble.h"

#include "Adventure.h"
#include "Logging.h"
#include "Menu.h"
#include "UILayers.h"
#include "Utils.h"

const char *UpdateFloorText(void)
{
	static char floorText[] = "00"; // Needs to be static because it's used by the system later.
	IntToString(floorText, 2, 0);
	return floorText;
}

int updateDelay = 0;

void SetUpdateDelay()
{
	updateDelay = 1;
}

bool adventureWindowVisible = false;

bool AdventureWindowIsVisible(void)
{
	return adventureWindowVisible;
}

void AdventureWindowAppear(Window *window);
void AdventureWindowDisappear(Window *window);

MenuDefinition adventureMenuDef = 
{
	.appear = AdventureWindowAppear,
	.disappear = AdventureWindowDisappear,
	.animated = true,
	.mainImageId = RESOURCE_ID_IMAGE_DUNGEONRIGHT
};

Window *adventureWindow = NULL;

void LoadRandomDungeonImage(void)
{
#if ALLOW_RANDOM_DUNGEON_GRAPHICS		
	int result;
#endif
	
#if ALLOW_RANDOM_DUNGEON_GRAPHICS		
	result = Random(12);
	if(result < 6)
		adventureMenuDef.mainImageId = RESOURCE_ID_IMAGE_DUNGEONSTRAIGHT;
	else if(result < 9)
		adventureMenuDef.mainImageId = RESOURCE_ID_IMAGE_DUNGEONLEFT;
	else if(result < 12)
		adventureMenuDef.mainImageId = RESOURCE_ID_IMAGE_DUNGEONRIGHT;
	else
		adventureMenuDef.mainImageId = RESOURCE_ID_IMAGE_DUNGEONDEADEND;
#endif

	if(adventureWindow)
		LoadMainBmpImage(adventureWindow, adventureMenuDef.mainImageId, -1);
}

void AdventureWindowAppear(Window *window)
{
	INFO_LOG("Back to the adventure.");
	DEBUG_LOG("Adventure appear floor %d",GetCurrentFloor());
	MenuAppear(window);
	ShowMainWindowRow(0, "Floor", UpdateFloorText());
	adventureWindow = window;
	SetUpdateDelay();
	UpdateClock();
	adventureWindowVisible = true;
}

void AdventureWindowDisappear(Window *window)
{
	DEBUG_LOG("Adventure disappear");
	adventureWindowVisible = false;
	MenuDisappear(window);
	adventureWindow = NULL;
}

void ShowAdventureWindow(void)
{
	INFO_LOG("Adventure Window");
	PushNewMenu(&adventureMenuDef);
}

typedef void (*ShowWindowFunction)(void);

typedef struct
{
	ShowWindowFunction windowFunction;
} RandomTableEntry;

void UpdateAdventure(void)
{
	if(!adventureWindowVisible)
		return;
	
	LoadRandomDungeonImage();
}
