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
#define ERROR_LOGGING 1 // ERROR_LOG should be used for actual incorrect operation.
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

// POSITIONING CONSTANTS

#define CLOCK_FRAME_WIDTH 62
#define CLOCK_FRAME_HEIGHT 36
#define CLOCK_TEXT_X_OFFSET 6
#define CLOCK_TEXT_Y_OFFSET -1
#define DATE_FRAME_WIDTH 60
#define DATE_FRAME_HEIGHT 18
#define DAY_FRAME_WIDTH DATE_FRAME_WIDTH
#define DAY_FRAME_HEIGHT DATE_FRAME_HEIGHT
#define BATTERY_HEIGHT 30
#define BATTERY_WIDTH 10
#define OK_FRAME_WIDTH 20
#define OK_FRAME_HEIGHT 22
#define MAIN_IMAGE_DIMENSION 84
#define MAIN_IMAGE_LAYER_W MAIN_IMAGE_DIMENSION
#define MAIN_IMAGE_LAYER_H MAIN_IMAGE_DIMENSION
#define MAIN_IMAGE_LAYER_Y 20

#if defined(PBL_ROUND)

#define SCREEN_WIDTH 180
#define SCREEN_HEIGHT 180

#define MAIN_IMAGE_LAYER_X (SCREEN_WIDTH - MAIN_IMAGE_LAYER_W - 20)

#define MAIN_MENU_FRAME_ON_SCREEN_X 95
#define MAIN_MENU_FRAME_Y_POS 48
#define MAIN_MENU_FRAME_WIDTH 75
#define MAIN_MENU_FRAME_HEIGHT 84

#define SLAVE_MENU_FRAME_ON_SCREEN_X 15
#define SLAVE_MENU_FRAME_Y_POS MAIN_MENU_FRAME_Y_POS
#define SLAVE_MENU_FRAME_WIDTH 80
#define SLAVE_MENU_FRAME_HEIGHT MAIN_MENU_FRAME_HEIGHT

#define LARGE_IMAGE_LAYER_W 104
#define LARGE_IMAGE_LAYER_H 104
#define LARGE_IMAGE_LAYER_X (SCREEN_WIDTH / 2 - LARGE_IMAGE_LAYER_W / 2)
#define LARGE_IMAGE_LAYER_Y (SCREEN_HEIGHT / 2 - LARGE_IMAGE_LAYER_H / 2)

#define LOCATION_PROGRESS_FRAME {.origin = {.x = 59, .y = 67}, .size = {.w = 16, .h = 46}}

#define BATTLE_FRAME_HEIGHT 40
#define BATTLE_FRAME_HEALTH_WIDTH 16
#define BATTLE_FRAME_TIME_WIDTH 8
#define BATTLE_FRAME_TIME_X (MAIN_IMAGE_LAYER_X - (BATTLE_FRAME_TIME_WIDTH + 2))
#define BATTLE_FRAME_HEALTH_X (BATTLE_FRAME_TIME_X - (BATTLE_FRAME_HEALTH_WIDTH + 2))
#define BATTLE_FRAME_ENEMY_Y (SCREEN_HEIGHT / 2 - MAIN_IMAGE_DIMENSION / 2)
#define BATTLE_FRAME_PLAYER_Y BATTLE_FRAME_ENEMY_Y + BATTLE_FRAME_HEIGHT + 4
#define PLAYER_HEALTH_FRAME {.origin = {.x = BATTLE_FRAME_HEALTH_X, .y = BATTLE_FRAME_PLAYER_Y}, .size = {.w = BATTLE_FRAME_HEALTH_WIDTH, .h = BATTLE_FRAME_HEIGHT}}
#define PLAYER_TIME_FRAME  {.origin = {.x = BATTLE_FRAME_TIME_X, .y = BATTLE_FRAME_PLAYER_Y}, .size = {.w = BATTLE_FRAME_TIME_WIDTH, .h = BATTLE_FRAME_HEIGHT}}
#define MONSTER_HEALTH_FRAME {.origin = {.x = BATTLE_FRAME_HEALTH_X, .y = BATTLE_FRAME_ENEMY_Y }, .size = {.w = BATTLE_FRAME_HEALTH_WIDTH, .h = BATTLE_FRAME_HEIGHT}}
#define MONSTER_TIME_FRAME {.origin = {.x = BATTLE_FRAME_TIME_X, .y = BATTLE_FRAME_ENEMY_Y}, .size = {.w = BATTLE_FRAME_TIME_WIDTH, .h = BATTLE_FRAME_HEIGHT}}

#elif defined(PBL_RECT)

#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168

#define MAIN_IMAGE_LAYER_X (SCREEN_WIDTH / 2 - MAIN_IMAGE_LAYER_W / 2)

#define OK_FRAME_WIDTH 20
#define OK_FRAME_HEIGHT 22

#define MAIN_MENU_FRAME_ON_SCREEN_X 72
#define MAIN_MENU_FRAME_Y_POS 42
#define MAIN_MENU_FRAME_WIDTH 72
#define MAIN_MENU_FRAME_HEIGHT 84

#define SLAVE_MENU_FRAME_ON_SCREEN_X 0
#define SLAVE_MENU_FRAME_Y_POS MAIN_MENU_FRAME_Y_POS
#define SLAVE_MENU_FRAME_WIDTH SCREEN_WIDTH - MAIN_MENU_FRAME_WIDTH
#define SLAVE_MENU_FRAME_HEIGHT MAIN_MENU_FRAME_HEIGHT

#define LARGE_IMAGE_LAYER_W 104
#define LARGE_IMAGE_LAYER_H 104
#define LARGE_IMAGE_LAYER_X (SCREEN_WIDTH - OK_FRAME_WIDTH - 2 - LARGE_IMAGE_LAYER_W)
#define LARGE_IMAGE_LAYER_Y (SCREEN_HEIGHT - CLOCK_FRAME_HEIGHT - 2 - LARGE_IMAGE_LAYER_H)

#define LOCATION_PROGRESS_FRAME_WIDTH 16
#define LOCATION_PROGRESS_FRAME_HEIGHT 84
#define LOCATION_PROGRESS_FRAME {.origin = {.x = SCREEN_WIDTH / 2 - 42 - LOCATION_PROGRESS_FRAME_WIDTH - 2, .y = SCREEN_HEIGHT / 2 - LOCATION_PROGRESS_FRAME_HEIGHT / 2}, .size = {.w = LOCATION_PROGRESS_FRAME_WIDTH, .h = LOCATION_PROGRESS_FRAME_HEIGHT}}

#define BATTLE_FRAME_HEIGHT 40
#define BATTLE_FRAME_HEALTH_WIDTH 16
#define BATTLE_FRAME_TIME_WIDTH 8
#define BATTLE_FRAME_TIME_X (MAIN_IMAGE_LAYER_X - (BATTLE_FRAME_TIME_WIDTH + 2))
#define BATTLE_FRAME_HEALTH_X (BATTLE_FRAME_TIME_X - (BATTLE_FRAME_HEALTH_WIDTH + 2))
#define BATTLE_FRAME_ENEMY_Y (SCREEN_HEIGHT / 2 - MAIN_IMAGE_DIMENSION / 2)
#define BATTLE_FRAME_PLAYER_Y BATTLE_FRAME_ENEMY_Y + BATTLE_FRAME_HEIGHT + 4
#define PLAYER_HEALTH_FRAME {.origin = {.x = BATTLE_FRAME_HEALTH_X, .y = BATTLE_FRAME_PLAYER_Y}, .size = {.w = BATTLE_FRAME_HEALTH_WIDTH, .h = BATTLE_FRAME_HEIGHT}}
#define PLAYER_TIME_FRAME  {.origin = {.x = BATTLE_FRAME_TIME_X, .y = BATTLE_FRAME_PLAYER_Y}, .size = {.w = BATTLE_FRAME_TIME_WIDTH, .h = BATTLE_FRAME_HEIGHT}}
#define MONSTER_HEALTH_FRAME {.origin = {.x = BATTLE_FRAME_HEALTH_X, .y = BATTLE_FRAME_ENEMY_Y }, .size = {.w = BATTLE_FRAME_HEALTH_WIDTH, .h = BATTLE_FRAME_HEIGHT}}
#define MONSTER_TIME_FRAME {.origin = {.x = BATTLE_FRAME_TIME_X, .y = BATTLE_FRAME_ENEMY_Y}, .size = {.w = BATTLE_FRAME_TIME_WIDTH, .h = BATTLE_FRAME_HEIGHT}}

#endif