#pragma once

#define BUILD_CLOUDPEBBLE_WORKER_APP 0

#if BUILD_CLOUDPEBBLE_WORKER_APP
#define BUILD_WORKER_FILES
#endif

#define WINDOW_ROW_HEIGHT 16

// Game tuning 
#define SKILL_DELAY 3

// Feature tuning
#define PAD_WITH_SPACES 1
#define DISABLE_MENU_BMPS 0

// Publish 0 turns on the test menu
#define PUBLISH 1
	
// Logging
#define DEBUG_LOGGING 0 // DEBUG_LOGGING 1 turns on DEBUG_LOG. DEBUG_LOGGING 2 turns on DEBUG_VERBOSE_LOG also.
#define ERROR_LOGGING 0 // ERROR_LOG should be used for actual incorrect operation.
#define WARNING_LOGGING 0 // WARNING_LOG should be used strange edge cases that are not expected
#define INFO_LOGGING 0 // INFO_LOG should be used to log game flow.

// Features to turn off to make space
#define BOUNDS_CHECKING 1

// Set up the test menu based on previous choices
#if PUBLISH
	#define ALLOW_TEST_MENU 0
#else
	#define ALLOW_TEST_MENU 1
#endif

#define TIME_TO_ACT 100

#define ALLOW_WORKER_APP 1
#define ALLOW_WORKER_APP_MESSAGES 1
#define ALLOW_WORKER_APP_LISTENING 1

typedef struct Window Window;

void ResetGame(void);
int HasFocus(void);
Window *GetBaseWindow(void);
