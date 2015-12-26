#pragma once

#define BUILD_CLOUDPEBBLE_WORKER_APP 0

#if BUILD_CLOUDPEBBLE_WORKER_APP
#define BUILD_WORKER_FILES
#endif

// Game tuning 
#define STAT_POINTS_PER_LEVEL 2
#define XP_FOR_NEXT_LEVEL 5 * level
#define SKILL_DELAY 3

// Feature tuning
#define PAD_WITH_SPACES 1
#define DISABLE_MENU_BMPS 0

// Publish 0 turns on the test menu
#define PUBLISH 1
	
// Stories to include
#define INCLUDE_DUNGEON_CRAWL 1
#define INCLUDE_DRAGON_QUEST 1
#define INCLUDE_BATTLE_TEST_STORY 1

#define INCLUDE_SLIDESHOW 0

typedef enum
{
	TITLE_MENU = 0,
	ADVENTURE_MODE,
	BATTLE_MODE,
} GameState;

enum
{
	DUNGEON_CRAWL_INDEX = 0,
	DRAGON_QUEST_INDEX = 1,
	BATTLE_TEST_INDEX = 2,
};

// Logging
#define DEBUG_LOGGING 0 // DEBUG_LOGGING 1 turns on DEBUG_LOG. DEBUG_LOGGING 2 turns on DEBUG_VERBOSE_LOG also.
#define ERROR_LOGGING 1 // ERROR_LOG should be used for actual incorrect operation.
#define WARNING_LOGGING 1 // WARNING_LOG should be used strange edge cases that are not expected
#define INFO_LOGGING 1 // INFO_LOG should be used to log game flow.
#define PROFILE_LOGGING 0 // PROFILE_LOGGING shows up as DEBUG_VERBOSE_LOG, but should be used for timing

// Features to turn off to make space
#define BOUNDS_CHECKING 1

// Set up the test menu based on previous choices
#if PUBLISH
	#define ALLOW_TEST_MENU 0
	#define FAST_MODE_IN_BACKGROUND false
#else
	#define ALLOW_TEST_MENU 1
	#define FAST_MODE_IN_BACKGROUND true
#endif

// This needs to be large enough to handle the maximum size of the window stack
#define MAX_MENU_WINDOWS 7
// This determines how many text rows there are in the interface
#define MAX_MENU_ENTRIES 6

#define MAX_EVENT_COUNT 10

#define TIME_TO_ACT 100
#define MAX_BATTLE_QUEUE 10
#define MAX_SKILLS_IN_LIST 15
	
#define ALLOW_WORKER_APP 0
#define ALLOW_WORKER_APP_MESSAGES 0
#define ALLOW_WORKER_APP_LISTENING 0

#define COMPILE_OLD_UILAYERS 0

typedef struct Window Window;

void ResetGame(void);
int HasFocus(void);
Window *GetBaseWindow(void);
