//
//  Story.c
//  
//
//  Created by Jonathan Panttaja on 1/25/16.
//
//
#include <pebble.h>
#include "AutoSizeConstants.h"
#include "Battler.h"
#include "BinaryResourceLoading.h"
#include "Character.h"
#include "DialogFrame.h"
#include "Location.h"
#include "Persistence.h"
#include "Story.h"
#include "StoryList.h"

typedef struct Story
{
    uint16_t id;
    uint16_t version;
    uint16_t hash;
    char name[MAX_STORY_NAME_LENGTH];
    char description[MAX_STORY_DESC_LENGTH];
    uint16_t start_location;
    uint16_t xpMonstersPerLevel;
    uint16_t xpDifferenceScale;
    uint16_t classCount;
    uint16_t classes[MAX_CLASSES];
    uint16_t openingDialog;
    uint16_t winDialog;
    uint16_t creditsDialog;
} Story;

static Story *Story_GetCurrentStory();

typedef struct StoryState
{
    bool needsSaving;
    PersistedStoryState persistedStoryState;
} StoryState;

static Story **storyList = NULL;

static int16_t currentStoryIndex = -1;
static uint16_t lastStoryId = 0;
static bool isLastStoryIdValid = false;
static StoryState currentStoryState = {0};

bool Story_IsLastStoryIdValid(void)
{
    return isLastStoryIdValid;
}

void Story_SetLastStoryId(uint16_t id)
{
    lastStoryId = id;
    isLastStoryIdValid = true;
}

uint16_t Story_GetLastStoryId(void)
{
    return lastStoryId;
}

static Story *Story_GetCurrentStory(void)
{
    if(currentStoryIndex < 0)
        return NULL;
    
    return storyList[currentStoryIndex];
}

uint16_t *Story_GetCurrentGameState(void)
{
    return currentStoryState.persistedStoryState.gameState;
}

void Story_UpdateGameState(uint16_t *stateChanges)
{
    for(int i = 0; i < MAX_GAME_STATE_VARIABLES; ++i)
    {
        currentStoryState.persistedStoryState.gameState[i] |= stateChanges[i];
    }
}

int16_t Story_GetStoryIndexById(uint16_t id)
{
    for(int i = 0; i < GetStoryCount(); ++i)
    {
        if(storyList[i]->id == id)
            return i;
    }
    return -1;
}

ResHandle Story_GetCurrentResHandle(void)
{
    if(currentStoryIndex < 0)
        return NULL;
    
    return resource_get_handle(GetStoryResourceIdByIndex(currentStoryIndex));
}

void Story_InitializeCurrent(void)
{
    currentStoryState.persistedStoryState.currentLocationIndex = Story_GetCurrentStory()->start_location;
    currentStoryState.persistedStoryState.timeOnPath = 0;
    for(int i = 0; i < MAX_GAME_STATE_VARIABLES; ++i)
    {
        currentStoryState.persistedStoryState.gameState[i] = 0;
    }
    
    Story_UpdateStoryWithPersistedState();
}

uint16_t Story_GetCurrentLocationIndex(void)
{
    return currentStoryState.persistedStoryState.currentLocationIndex;
}

bool Story_IncrementTimeOnPath(void)
{
    currentStoryState.persistedStoryState.timeOnPath++;
    return currentStoryState.persistedStoryState.timeOnPath >= Location_GetCurrentLength();
}

uint16_t Story_GetTimeOnPath(void)
{
    return currentStoryState.persistedStoryState.timeOnPath;
}

StoryUpdateReturnType Story_UpdateCurrentLocation(void)
{
    if(Location_CurrentLocationIsPath())
    {
        bool pathEnded = Story_IncrementTimeOnPath();
        if(pathEnded)
        {
            return Story_MoveToLocation(currentStoryState.persistedStoryState.destinationIndex);
        }
        else
        {
            return STORYUPDATE_COMPUTERANDOM;
        }
    }
    
    return STORYUPDATE_DONOTHING;
}

StoryUpdateReturnType Story_MoveToLocation(uint16_t index)
{
    uint16_t globalIndex = 0;
    if((globalIndex = Location_MoveToAdjacentLocation(index)))
    {
        uint16_t oldIndex = currentStoryState.persistedStoryState.currentLocationIndex;
        currentStoryState.persistedStoryState.currentLocationIndex = globalIndex;
        currentStoryState.persistedStoryState.timeOnPath = 0;
        currentStoryState.persistedStoryState.encounterChance = Location_GetCurrentEncounterChance();
        currentStoryState.persistedStoryState.pathLength = Location_GetCurrentLength();
        
        if(Location_CurrentLocationIsRestArea())
        {
            Character_Rest();
        }
        
        if(Location_CurrentLocationIsLevelUp())
        {
            Character_GrantLevel();
        }
        
        if(Location_CurrentLocationEndsGame())
        {
            return STORYUPDATE_WIN;
        }
        
        if(Location_CurrentLocationIsPath())
        {
            if(Location_GetCurrentAdjacentLocationByIndex(0) == oldIndex)
                currentStoryState.persistedStoryState.destinationIndex = 1;
            else
                currentStoryState.persistedStoryState.destinationIndex = 0;
        }
        else
        {
            if(Location_CurrentLocationHasMonster())
            {
                return STORYUPDATE_TRIGGER_BATTLE;
            }
        }
        return STORYUPDATE_FULLREFRESH;
    }
    return STORYUPDATE_DONOTHING;
}

void Story_FreeAll(void)
{
    for(int i = 0; i < GetStoryCount(); ++i)
    {
        free(storyList[i]);
    }
    free(storyList);
}

static Story *Story_Load(int resourceId)
{
    ResHandle currentStoryData = resource_get_handle(resourceId);
    Story *currentStory = calloc(sizeof(Story), 1);
    ResourceLoadStruct(currentStoryData, 0, (uint8_t*)currentStory, sizeof(Story), "Story");
    return currentStory;
}

#if DEBUG_LOGGING > 1
static void Story_Log(Story *story)
{
    DEBUG_VERBOSE_LOG("Story: %s, %s, %d, %d, %d", story->name, story->description, story->id, story->version, story->start_location);
}
#endif

void Story_LoadAll(void)
{
    if(storyList)
        return;
    
    storyList = calloc(sizeof(Story*), GetStoryCount());
    
    for(int i = 0; i < GetStoryCount(); ++i)
    {
        storyList[i] = Story_Load(GetStoryResourceIdByIndex(i));
#if DEBUG_LOGGING > 1
        Story_Log(storyList[i]);
#endif
    }
}

void Story_SetCurrentStory(uint16_t index)
{
    currentStoryIndex = index;
    Story_SetLastStoryId(storyList[currentStoryIndex]->id);
}

void Story_ClearCurrentStory(void)
{
    currentStoryIndex = -1;
    Location_ClearCurrentLocation();
}

void Story_LogCurrent(void)
{
#if DEBUG_LOGGING
    Story *currentStory = Story_GetCurrentStory();
    DEBUG_LOG("Story: %s, %s, %d, %d, %d", currentStory->name, currentStory->description, currentStory->id, currentStory->version, currentStory->start_location);
#endif
}

const char *Story_GetNameByIndex(uint16_t index)
{
    if(index >= GetStoryCount())
        return "None";
    
    Story *story = storyList[index];
    return story->name;
}

const char *Story_GetDescriptionByIndex(uint16_t index)
{
    if(index >= GetStoryCount())
        return "None";
    
    Story *story = storyList[index];
    return story->description;
}

bool Story_InStory(void)
{
    return currentStoryIndex != -1;
}

void Story_GetStoryList(uint16_t *count, uint16_t **buffer)
{
    *count = GetStoryCount();
    *buffer = calloc(sizeof(uint16_t), *count);
    
    for(int i = 0; i < *count; ++i)
    {
        (*buffer)[i] = storyList[i]->id;
    }
}

uint16_t Story_GetCurrentStoryId(void)
{
    return Story_GetCurrentStory()->id;
}

uint16_t Story_GetCurrentStoryVersion(void)
{
    return Story_GetCurrentStory()->version;
}

uint16_t Story_GetCurrentStoryHash(void)
{
    return Story_GetCurrentStory()->hash;
}

uint16_t Story_GetCurrentStoryXPMonstersPerLevel(void)
{
    return Story_GetCurrentStory()->xpMonstersPerLevel;
}

uint16_t Story_GetCurrentStoryXPDifferenceScale(void)
{
    return Story_GetCurrentStory()->xpDifferenceScale;
}

uint16_t Story_GetClassByIndex(uint16_t index)
{
    return Story_GetCurrentStory()->classes[index];
}

void Story_GetPersistedData(uint16_t *count, uint8_t **buffer)
{
    *count = sizeof(currentStoryState.persistedStoryState);
    *buffer = (uint8_t*)(&currentStoryState.persistedStoryState);
}

void Story_UpdateStoryWithPersistedState(void)
{
    Location_SetCurrentLocation(currentStoryState.persistedStoryState.currentLocationIndex);
    currentStoryState.persistedStoryState.encounterChance = Location_GetCurrentEncounterChance();
    currentStoryState.persistedStoryState.pathLength = Location_GetCurrentLength();
    Battler_UnloadPlayer();
}

void Story_QueueCreditsDialog(void)
{
    Dialog_QueueFromResource(Story_GetCurrentResHandle(), Story_GetCurrentStory()->creditsDialog);
}

void Story_TriggerWinDialog(void)
{
    Dialog_TriggerFromResource(Story_GetCurrentResHandle(), Story_GetCurrentStory()->winDialog);
}

void Story_TriggerOpeningDialog(void)
{
    Dialog_TriggerFromResource(Story_GetCurrentResHandle(), Story_GetCurrentStory()->openingDialog);
}

