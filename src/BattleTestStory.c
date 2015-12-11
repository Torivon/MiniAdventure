#include <pebble.h>
#include "BattleTestStory.h"

#include "pebble.h"

#include "Adventure.h"
#include "Character.h"
#include "DungeonCrawl.h"
#include "Items.h"
#include "Location.h"
#include "LocationInternal.h"
#include "Logging.h"
#include "MiniAdventure.h"
#include "Monsters.h"
#include "NewBattle.h"
#include "Story.h"
	
#if INCLUDE_BATTLE_TEST_STORY

enum
{
	ARENA = 0,
	RAT_ROOM,
};

enum
{
	DUNGEON_RAT = 0,
	DUNGEON_GOBLIN,
	DUNGEON_WIZARD,
	DUNGEON_ZOMBIE,
	DUNGEON_TURTLE,
	DUNGEON_LICH,
	DUNGEON_DRAGON,
};

static FixedClass ArenaClass =
{
	.backgroundImage = RESOURCE_ID_IMAGE_DUNGEONDEADEND,
	.allowShop = false,
};

static FixedClass RatClass =
{
	.monster = DUNGEON_RAT,
	.backgroundImage = RESOURCE_ID_IMAGE_DUNGEONDEADEND,
};

static MonsterDef monsters[] =
{
	{
		.name = "Rat",
		.imageId = RESOURCE_ID_IMAGE_RAT,
		.extraFireDefenseMultiplier = 100,
		.extraIceDefenseMultiplier = 100,
		.extraLightningDefenseMultiplier = 100,
		.powerLevel = 0,
		.healthLevel = 0,
		.defenseLevel = 0,
		.magicDefenseLevel = 0,
		.allowPhysicalAttack = true,
		.goldScale = 1,
		.speed = 20,
	},

	{
		.name = "Goblin",
		.imageId = RESOURCE_ID_IMAGE_GOBLIN,
		.extraFireDefenseMultiplier = 100,
		.extraIceDefenseMultiplier = 200,
		.extraLightningDefenseMultiplier = 100,
		.powerLevel = 1,
		.healthLevel = 1,
		.defenseLevel = 1,
		.magicDefenseLevel = 0,
		.allowPhysicalAttack = true,
		.goldScale = 2
	},

	{
		.name = "Wizard",
		.imageId = RESOURCE_ID_IMAGE_WIZARD,
		.extraFireDefenseMultiplier = 100,
		.extraIceDefenseMultiplier = 100,
		.extraLightningDefenseMultiplier = 100,
		.powerLevel = 1,
		.healthLevel = 1,
		.defenseLevel = 1,
		.magicDefenseLevel = 3,
		.allowMagicAttack = true,
		.goldScale = 3
	},

	{
		.name = "Zombie",
		.imageId = RESOURCE_ID_IMAGE_ZOMBIE,
		.extraFireDefenseMultiplier = 300,
		.extraIceDefenseMultiplier = 100,
		.extraLightningDefenseMultiplier = 100,
		.powerLevel = 2,
		.healthLevel = 1,
		.defenseLevel = 1,
		.magicDefenseLevel = 1,
		.allowPhysicalAttack = true,
		.goldScale = 2
	},

	{
		.name = "Turtle",
		.imageId = RESOURCE_ID_IMAGE_TURTLE,
		.extraFireDefenseMultiplier = 100,
		.extraIceDefenseMultiplier = 100,
		.extraLightningDefenseMultiplier = 600,
		.powerLevel = 1,
		.healthLevel = 2,
		.defenseLevel = 3,
		.magicDefenseLevel = 1,
		.allowPhysicalAttack = true,
		.goldScale = 1
	},

	{
		.name = "Lich",
		.imageId = RESOURCE_ID_IMAGE_LICH,
		.extraFireDefenseMultiplier = 100,
		.extraIceDefenseMultiplier = 600,
		.extraLightningDefenseMultiplier = 100,
		.powerLevel = 2,
		.healthLevel = 2,
		.defenseLevel = 3,
		.magicDefenseLevel = 1,
		.allowMagicAttack = true,
		.goldScale = 5
	},

	{
		.name = "Dragon",
		.imageId = RESOURCE_ID_IMAGE_DRAGON,
		.extraFireDefenseMultiplier = 100,
		.extraIceDefenseMultiplier = 100,
		.extraLightningDefenseMultiplier = 100,
		.powerLevel = 2,
		.healthLevel = 2,
		.defenseLevel = 2,
		.magicDefenseLevel = 2,
		.allowPhysicalAttack = true,
		.allowMagicAttack = true,
		.goldScale = 10,
		.preventRun = true,
	},
};

static Location locationList[] =
{
	{ //ARENA
		.name = "Arena",
		.type = LOCATIONTYPE_FIXED,
		.numberOfAdjacentLocations = 1,
		.adjacentLocations = {RAT_ROOM},
		.fixedclass = &ArenaClass,
	},
	{ //RAT_ROOM
		.name = "Rat Room",
		.type = LOCATIONTYPE_FIXED,
		.numberOfAdjacentLocations = 1,
		.adjacentLocations = {ARENA},
		.fixedclass = &RatClass,
		.baseLevel = 1,
		.fixed_ArrivalFunction = ShowNewBattleWindow,
	},
};

StoryState battleTestStoryState = {0};

void InitializeBattleTest(void)
{
	battleTestStoryState.needsSaving = true;
	battleTestStoryState.persistedStoryState.currentLocationIndex = 0;
	battleTestStoryState.persistedStoryState.currentLocationDuration = 0;
	battleTestStoryState.persistedStoryState.currentPathDestination = 0;
	battleTestStoryState.persistedStoryState.mostRecentMonster = 0;
	InitializeCharacter();
}

Story battleTestStory = 
{
	.gameNumber = BATTLE_TEST_INDEX,
	.gameDataVersion = 1,
	.locationList = locationList,
	.monsterList = monsters,
	.initializeStory = InitializeBattleTest,
};

void LaunchBattleTestStory(void)
{
	battleTestStory.numberOfLocations = sizeof(locationList)/sizeof(Location);
	battleTestStory.numberOfMonsters = sizeof(monsters)/sizeof(MonsterDef);
	RegisterStory(&battleTestStory, &battleTestStoryState);
	DEBUG_LOG("Initialized locationList size = %d", sizeof(locationList));
	ShowAdventureWindow();
	
}

#endif