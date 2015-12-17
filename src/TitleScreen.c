#include "pebble.h"

#include "Adventure.h"
#include "DescriptionFrame.h"
#include "DialogFrame.h"
#include "GlobalState.h"
#include "Logging.h"
#include "MainImage.h"
#include "MainMenu.h"
#include "Menu.h"
#include "MiniAdventure.h"
#include "OptionsMenu.h"
#include "NewBaseWindow.h"
#include "Slideshow.h"

#include "DungeonCrawl.h"
#include "DragonQuest.h"
#include "BattleTestStory.h"

#include "NewMenu.h"

enum 
{
	TITLE_STORY_NONE = 0,
	TITLE_DUNGEON_CRAWL,
	TITLE_DRAGON_QUEST,
	TITLE_BATTLE_TEST,
	TITLE_SLIDESHOW,
	TITLE_OPTIONS,
	TITLE_CREDITS
};

static int gameToLaunch = TITLE_STORY_NONE;

void ChooseDungeonCrawl(void)
{
	gameToLaunch = TITLE_DUNGEON_CRAWL;
}

void ChooseDragonQuest(void)
{
	gameToLaunch = TITLE_DRAGON_QUEST;
}

void ChooseBattleTest(void)
{
	gameToLaunch = TITLE_BATTLE_TEST;
}

void ChooseSlideshow(void)
{
	gameToLaunch = TITLE_SLIDESHOW;
}

void ChooseOptions(void)
{
	gameToLaunch = TITLE_OPTIONS;
}

void ChooseCredits(void)
{
	gameToLaunch = TITLE_CREDITS;
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
	{.name = "Battle Test", .description = "Battle arena", .callback = ChooseBattleTest},
#endif
#if INCLUDE_SLIDESHOW
	{.name = "Slideshow", .description = "Slideshow of all art", .callback = ChooseSlideshow},
#endif
	{.name = "Options", .description = "Options", .callback = ChooseOptions},
	{.name = "Credits", .description = "Credits", .callback = ChooseCredits},
};

static void TitleScreenAppear(void *data)
{
	RegisterMenuCellList(GetMainMenu(), titleScreenMenuList, sizeof(titleScreenMenuList)/sizeof(*titleScreenMenuList));
	SetForegroundImage(RESOURCE_ID_IMAGE_TITLE);
	SetMainImageVisibility(true, true, false);
	SetDescription("MiniAdventure");
		
	switch(gameToLaunch)
	{
		case TITLE_DUNGEON_CRAWL:
		{
#if INCLUDE_DUNGEON_CRAWL
			LaunchDungeonCrawl();
#endif
			break;
		}
		case TITLE_DRAGON_QUEST:
		{
#if INCLUDE_DRAGON_QUEST
			LaunchDragonQuest();
#endif
			break;
		}
		case TITLE_BATTLE_TEST:
		{
#if INCLUDE_BATTLE_TEST_STORY
			LaunchBattleTestStory();
#endif
			break;
		}
		case TITLE_SLIDESHOW:
		{
#if INCLUDE_SLIDESHOW
			LaunchSlideshow();
#endif
			break;
		}
		case TITLE_OPTIONS:
		{
			TriggerOptionScreen();
			break;
		}
		case TITLE_CREDITS:
		{
			TriggerDialog("Programming and art by Jonathan Panttaja, with help from Belphemur and BlackLamb");
			break;
		}
		default:
		{
			break;
		}
	}
	gameToLaunch = TITLE_STORY_NONE;
}

static void TitleScreenPop(void *data)
{
	SetMainImageVisibility(false, false, false);
	SetDescription("");
}

void RegisterTitleScreen(void)
{
	PushGlobalState(STATE_TITLE_SCREEN, 0, NULL, NULL, TitleScreenAppear, NULL, TitleScreenPop, NULL);
}
