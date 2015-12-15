#include "pebble.h"

#include "Adventure.h"
#include "DescriptionFrame.h"
#include "GlobalState.h"
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

#include "NewMenu.h"

enum 
{
	STORY_NONE = 0,
	DUNGEON_CRAWL,
	DRAGON_QUEST,
	BATTLE_TEST,
	SLIDESHOW,
};

static int gameToLaunch = STORY_NONE;

void ChooseDungeonCrawl(void)
{
	gameToLaunch = DUNGEON_CRAWL;
}

void ChooseDragonQuest(void)
{
	gameToLaunch = DRAGON_QUEST;
}

void ChooseBattleTest(void)
{
	gameToLaunch = BATTLE_TEST;
}

void ChooseSlideshow(void)
{
	gameToLaunch = SLIDESHOW;
}

MenuCellDescription titleScreenMenuList[] = 
{
#if INCLUDE_DUNGEON_CRAWL
	{.name = "Dungeon", .description = "Simple dungeon delve", .callback = ChooseDungeonCrawl},
#endif
#if INCLUDE_DRAGON_QUEST
	{.name = "Dragon Quest", .description = "Extended adventure", .callback = ChooseDragonQuest},
#endif
#if INCLUDE_BATTLE_TEST_STORY
	{.name = "Battle Test", .description = "Battle arena", .callback = ChooseBattleTest}
#endif
#if INCLUDE_SLIDESHOW
	{.name = "Slideshow", .description = "Slideshow of all art", .callback = ChooseSlideshow}
#endif
};

static void TitleScreenAppear(void)
{
	RegisterMenuCellList(titleScreenMenuList, sizeof(titleScreenMenuList)/sizeof(*titleScreenMenuList));
	SetForegroundImage(RESOURCE_ID_IMAGE_TITLE);
	SetMainImageVisibility(true, true, false);
	SetDescription("MiniAdventure");
	
	switch(gameToLaunch)
	{
		case DUNGEON_CRAWL:
		{
#if INCLUDE_DUNGEON_CRAWL
			LaunchDungeonCrawl();
#endif
			break;
		}
		case DRAGON_QUEST:
		{
#if INCLUDE_DRAGON_QUEST
			LaunchDragonQuest();
#endif
			break;
		}
		case BATTLE_TEST:
		{
#if INCLUDE_BATTLE_TEST_STORY
			LaunchBattleTestStory();
#endif
			break;
		}
		case SLIDESHOW:
		{
#if INCLUDE_SLIDESHOW
			LaunchSlideshow();
#endif
			break;
		}
		default:
		{
			break;
		}
	}
	gameToLaunch = STORY_NONE;
}

static void TitleScreenPop(void)
{
	SetMainImageVisibility(false, false, false);
	SetDescription("");
}

void RegisterTitleScreen(void)
{
	PushGlobalState(TITLE_SCREEN, 0, NULL, NULL, TitleScreenAppear, NULL, TitleScreenPop);
}