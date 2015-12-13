#include "pebble.h"

#include "Adventure.h"
#include "Logging.h"
#include "MainImage.h"
#include "MainMenu.h"
#include "Menu.h"
#include "MiniAdventure.h"
#include "Slideshow.h"
#include "UILayers.h"

#include "DungeonCrawl.h"
#include "DragonQuest.h"
#include "BattleTestStory.h"

#if USE_MENULAYER_PROTOTYPE

#include "NewMenu.h"

MenuCellDescription titleMenuList[] = 
{
#if INCLUDE_DUNGEON_CRAWL
	{.name = "Dungeon", .callback = LaunchDungeonCrawl},
#endif
#if INCLUDE_DRAGON_QUEST
	{.name = "Dragon Quest", .callback = LaunchDragonQuest},
#endif
#if INCLUDE_BATTLE_TEST_STORY
	{.name = "Battle Test", .callback = LaunchBattleTestStory}
#endif
#if INCLUDE_SLIDESHOW
	{.name = "Slideshow", .callback = LaunchSlideshow}
#endif
};

void RegisterTitleMenu(void)
{
	RegisterMenuCellList(titleMenuList, sizeof(titleMenuList)/sizeof(*titleMenuList));
	SetForegroundImage(RESOURCE_ID_IMAGE_TITLE);
	SetMainImageVisibility(true, true, false);	
}

#else
void TitleMenuWindowAppear(Window *window)
{
	MenuAppear(window);
	ShowMainWindowRow(0, "MiniAdventure", "");
}

static MenuDefinition titleMenuDef = 
{
	.menuEntries = 
	{
		{.text = NULL, .description = NULL, .menuFunction = NULL},
#if INCLUDE_DUNGEON_CRAWL
		{.text = "Dungeon", .description = "A simple dungeon crawl", .menuFunction = LaunchDungeonCrawl},
#else
		{.text = NULL, .description = NULL, .menuFunction = NULL},
#endif
#if INCLUDE_DRAGON_QUEST
		{.text = "Quest", .description = "Quest to kill a dragon", .menuFunction = LaunchDragonQuest},
#else
		{.text = NULL, .description = NULL, .menuFunction = NULL},
#endif
#if INCLUDE_BATTLE_TEST_STORY
		{.text = "Battle test", .description = "Test new battle system", .menuFunction = LaunchBattleTestStory},
#else
		{.text = NULL, .description = NULL, .menuFunction = NULL},
#endif
#if INCLUDE_SLIDESHOW
		{.text = "Slideshow", .description = "Slideshow of all art", .menuFunction = LaunchSlideshow},
#else
		{.text = NULL, .description = NULL, .menuFunction = NULL},
#endif
	},
	.appear = TitleMenuWindowAppear,
	.mainImageId = RESOURCE_ID_IMAGE_TITLE,
	.floorImageId = -1
};

void ShowTitleMenu(void)
{
	INFO_LOG("Entering title menu.");
	PushNewMenu(&titleMenuDef);
}
#endif