//
//  Story.h
//  
//
//  Created by Jonathan Panttaja on 1/25/16.
//
//
#pragma once

typedef enum
{
    STORYUPDATE_COMPUTERANDOM = 0,
    STORYUPDATE_DONOTHING,
    STORYUPDATE_FULLREFRESH,
    STORYUPDATE_WIN,
    STORYUPDATE_TRIGGER_BATTLE,
    STORYUPDATE_ENDPATH,
    STORYUPDATE_SKIP_ENCOUNTER_WITH_XP,
    STORYUPDATE_SKIP_ENCOUNTER_NO_XP
} StoryUpdateReturnType;

typedef struct Story Story;
typedef struct EventStateChange EventStateChange;

ResHandle Story_GetCurrentResHandle(void);

void Story_GetStoryList(uint16_t *count, uint16_t **buffer);
int16_t Story_GetStoryIndexById(uint16_t id);
void Story_GetPersistedData(uint16_t *count, uint8_t **buffer);
void Story_UpdateStoryWithPersistedState(void);

void Story_InitializeCurrent(void);
void Story_SetCurrentStory(uint16_t index);
void Story_ClearCurrentStory(void);

bool Story_IsLastStoryIdValid(void);
void Story_SetLastStoryId(uint16_t id);
uint16_t Story_GetLastStoryId(void);

const char *Story_GetNameByIndex(uint16_t index);
const char *Story_GetDescriptionByIndex(uint16_t index);
uint16_t Story_GetTimeOnPath(void);
uint16_t Story_GetCurrentLocationLength(void);
uint16_t Story_GetClassByIndex(uint16_t index);

StoryUpdateReturnType Story_UpdateCurrentLocation(void);
StoryUpdateReturnType Story_MoveToAdjacentLocation(uint16_t index);
StoryUpdateReturnType Story_MoveToRespawnPoint(void);
uint16_t Story_GetCurrentStoryId(void);
uint16_t Story_GetCurrentStoryVersion(void);
uint16_t Story_GetCurrentStoryHash(void);
uint16_t Story_GetCurrentStoryXPMonstersPerLevel(void);
uint16_t Story_GetCurrentStoryXPDifferenceScale(void);
bool Story_GetCurrentStoryAllowRespawn(void);
bool Story_GetCurrentGameStateValue(uint16_t bit);
uint16_t *Story_GetCurrentGameState(void);
void Story_UpdateGameState(EventStateChange *stateChanges);

void Story_QueueCreditsDialog(void);
void Story_TriggerWinDialog(void);
void Story_TriggerOpeningDialog(void);

void Story_LoadAll(void);
void Story_FreeAll(void);

void Story_GetCurrentKeyItemList(uint16_t *count, uint16_t **list);
