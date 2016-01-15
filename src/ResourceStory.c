#include "pebble.h"

#include "ImageMap.h"
#include "AutoSizeConstants.h"
#include "AutoLocationConstants.h"
#include "BinaryResourceLoading.h"
#include "Character.h"
#include "CombatantClass.h"
#include "DialogFrame.h"
#include "GlobalState.h"
#include "Utils.h"
#include "Logging.h"
#include "ResourceStory.h"
#include "Skills.h"
#include "StoryList.h"


/********************* PREDECLARATIONS *********************************/

typedef struct ResourceStory
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
} ResourceStory;

static ResHandle ResourceStory_GetCurrentResHandle(void);
static ResourceStory *ResourceStory_GetCurrentStory();

typedef struct ResourceStoryState
{
    bool needsSaving;
    PersistedResourceStoryState persistedResourceStoryState;
} ResourceStoryState;

static int16_t currentResourceStoryIndex = -1;
uint16_t lastResourceStoryId = 0;
bool isLastResourceStoryIdValid = false;
static ResourceStoryState currentResourceStoryState = {0};

typedef struct ResourceLocation
{
    char name[MAX_STORY_NAME_LENGTH];
    uint16_t adjacentLocationCount;
    uint16_t adjacentLocations[MAX_ADJACENT_LOCATIONS];
    uint16_t backgroundImageCount;
    uint16_t backgroundImages[MAX_BACKGROUND_IMAGES];
    uint16_t locationProperties;
    uint16_t length;
    uint16_t baseLevel;
    uint16_t encounterChance;
    uint16_t monsterCount;
    uint16_t monsters[MAX_MONSTERS];
    uint16_t initialEvent;
    uint16_t localEventCount;
    uint16_t localEvents[MAX_EVENTS];
} ResourceLocation;

static ResourceLocation *currentLocation = NULL;
static ResourceEvent *currentLocationInitialEvent = NULL;
static ResourceLocation *adjacentLocations[MAX_ADJACENT_LOCATIONS] = {0};
static ResourceEvent *adjacentLocationInitialEvents[MAX_ADJACENT_LOCATIONS] = {0};
static ResourceEvent *localEvents[MAX_EVENTS] = {0};

/********************* RESOURCE EVENT *******************************/

typedef struct ResourceEvent
{
    char name[MAX_STORY_NAME_LENGTH];
    uint16_t dialog;
    uint16_t usePrerequisites;
    uint16_t positivePrerequisites[MAX_GAME_STATE_VARIABLES];
    uint16_t negativePrerequisites[MAX_GAME_STATE_VARIABLES];
    uint16_t stateChanges[MAX_GAME_STATE_VARIABLES];
} ResourceEvent;

ResourceEvent *ResourceEvent_Load(uint16_t logical_index)
{
    if(logical_index == 0)
        return NULL;
    
    ResourceEvent *event = calloc(sizeof(ResourceEvent), 1);
    ResourceLoadStruct(ResourceStory_GetCurrentResHandle(), logical_index, (uint8_t*)event, sizeof(ResourceEvent), "ResourceEvent");
    return event;
}

void ResourceEvent_Free(ResourceEvent *event)
{
    if(event)
        free(event);
}

static bool CheckPrerequisites(uint16_t *gameStateList, uint16_t *prerequisiteList, bool positive)
{
    bool match = true;
    for(int i = 0; i < MAX_GAME_STATE_VARIABLES; ++i)
    {
        uint16_t gameState  = gameStateList[i];
        uint16_t prerequisite = prerequisiteList[i];
        if(prerequisite > 0)
        {
            bool result = ((gameState & prerequisite) == prerequisite);
            if(!positive)
                result = !result;
            match = match && result;
        }
    }
    return match;
}

bool ResourceEvent_CheckPrerequisites(ResourceEvent *event)
{
    if(!event || !event->usePrerequisites)
        return true;
    
    bool match = true;
    match = match && CheckPrerequisites(currentResourceStoryState.persistedResourceStoryState.gameState, event->positivePrerequisites, true);
    match = match && CheckPrerequisites(currentResourceStoryState.persistedResourceStoryState.gameState, event->negativePrerequisites, false);
    return match;
}

void ResourceEvent_UpdateGameState(void *data)
{
    uint16_t *stateChanges = (uint16_t*)data;
    for(int i = 0; i < MAX_GAME_STATE_VARIABLES; ++i)
    {
        currentResourceStoryState.persistedResourceStoryState.gameState[i] |= stateChanges[i];
    }
}

void ResourceEvent_UpdateGameState_Push(void *data)
{
    ResourceEvent *event = (ResourceEvent*)data;
    ResourceEvent_UpdateGameState(event);
    GlobalState_Pop();
}

void ResourceEvent_TriggerEvent(ResourceEvent *event, bool now)
{
    if(event->dialog > 0)
    {
        if(now)
            ResourceStory_TriggerDialog(event->dialog);
        else
            ResourceStory_QueueDialog(event->dialog);
        GlobalState_Queue(STATE_UPDATE_GAME_STATE, 0, event->stateChanges);
    }
    else
    {
        if(now)
            GlobalState_Push(STATE_UPDATE_GAME_STATE, 0, event->stateChanges);
        else
            GlobalState_Queue(STATE_UPDATE_GAME_STATE, 0, event->stateChanges);
    }
}

void ResourceEvent_Trigger(uint16_t index)
{
    uint16_t newIndex = 0;
    uint16_t indexToUse = 0;
    for(int i = 0; i < currentLocation->localEventCount; ++i)
    {
        if(ResourceEvent_CheckPrerequisites(localEvents[i]))
        {
            if(newIndex == index)
            {
                indexToUse = i;
                break;
            }
            ++newIndex;
        }
    }
    
    if(localEvents[indexToUse]->dialog > 0)
    {
        ResourceStory_TriggerDialog(localEvents[indexToUse]->dialog);
        GlobalState_Queue(STATE_UPDATE_GAME_STATE, 0, localEvents[indexToUse]->stateChanges);
    }
    else
    {
        GlobalState_Push(STATE_UPDATE_GAME_STATE, 0, localEvents[indexToUse]->stateChanges);
    }
}

void ResourceEvent_Queue(uint16_t index)
{
    uint16_t newIndex = 0;
    uint16_t indexToUse = 0;
    for(int i = 0; i < currentLocation->localEventCount; ++i)
    {
        if(ResourceEvent_CheckPrerequisites(localEvents[i]))
        {
            if(newIndex == index)
            {
                indexToUse = i;
                break;
            }
            ++newIndex;
        }
    }

    if(localEvents[indexToUse]->dialog > 0)
        ResourceStory_QueueDialog(localEvents[indexToUse]->dialog);
    GlobalState_Queue(STATE_UPDATE_GAME_STATE, 0, localEvents[indexToUse]->stateChanges);
}

/********************* RESOURCE DIALOG *******************************/

DialogData *ResourceDialog_Load(uint16_t dialogIndex)
{
    if(dialogIndex == 0)
        return NULL;
    
    DialogData *dialog = calloc(sizeof(DialogData), 1);
    ResourceLoadStruct(ResourceStory_GetCurrentResHandle(), dialogIndex, (uint8_t*)dialog, sizeof(DialogData), "DialogData");
    return dialog;
}

void ResourceStory_TriggerDialog(uint16_t dialogIndex)
{
    DialogData *dialog = ResourceDialog_Load(dialogIndex);
    if(!dialog)
        return;
    TriggerDialog(dialog);
}

void ResourceStory_QueueDialog(uint16_t dialogIndex)
{
    DialogData *dialog = ResourceDialog_Load(dialogIndex);
    if(!dialog)
        return;
    QueueDialog(dialog);
}

uint16_t ResourceStory_GetOpeningDialogIndex(void)
{
    return ResourceStory_GetCurrentStory()->openingDialog;
}

uint16_t ResourceStory_GetWinDialogIndex(void)
{
    return ResourceStory_GetCurrentStory()->winDialog;
}

/********************* RESOURCE LOCATION *******************************/

static ResourceLocation *ResourceLocation_Load(uint16_t logical_index)
{
    ResHandle currentStoryData = ResourceStory_GetCurrentResHandle();
    ResourceLocation *newLocation = calloc(sizeof(ResourceLocation), 1);
    ResourceLoadStruct(currentStoryData, logical_index, (uint8_t*)newLocation, sizeof(ResourceLocation), "ResourceLocation");
    
    for(int i = 0; i < newLocation->backgroundImageCount; ++i)
    {
        newLocation->backgroundImages[i] = ImageMap_GetIdByIndex(newLocation->backgroundImages[i]);
    }
    
    return newLocation;
}

uint16_t ResourceStory_GetCurrentLocationEncounterChance(void)
{
    if(currentLocation)
    {
        return currentLocation->encounterChance;
    }
    else
    {
        return 0;
    }
}

uint16_t ResourceStory_GetCurrentLocationBaseLevel(void)
{
    if(currentLocation)
    {
        return currentLocation->baseLevel;
    }
    else
    {
        return 1;
    }
}

static void ResourceLocation_Free(ResourceLocation *location)
{
    if(location)
    {
        free(location);
    }
}

#if DEBUG_LOGGING > 1
static void ResourceLocation_Log(ResourceLocation *location)
{
    DEBUG_VERBOSE_LOG("ResourceLocation: %s", location->name);
    for(int i = 0; i < location->adjacentLocationCount; ++i)
    {
        DEBUG_VERBOSE_LOG("Adjacent Location: %d", location->adjacentLocations[i]);
    }
    for(int i = 0; i < location->backgroundImageCount; ++i)
    {
        DEBUG_VERBOSE_LOG("Background: %d", location->backgroundImages[i]);
    }
}
#endif

static void ResourceLocation_LoadAdjacentLocations(void)
{
    for(int i = 0; i < currentLocation->adjacentLocationCount; ++i)
    {
        adjacentLocations[i] = ResourceLocation_Load(currentLocation->adjacentLocations[i]);
        adjacentLocationInitialEvents[i] = ResourceEvent_Load(adjacentLocations[i]->initialEvent);
    }
}

void ResourceLocation_FreeAdjacentLocations(void)
{
    for(int i = 0; i < MAX_ADJACENT_LOCATIONS; ++i)
    {
        if(adjacentLocations[i])
        {
            ResourceLocation_Free(adjacentLocations[i]);
            adjacentLocations[i] = NULL;
            ResourceEvent_Free(adjacentLocationInitialEvents[i]);
            adjacentLocationInitialEvents[i] = NULL;
        }
    }
}

static void ResourceLocation_LoadLocalEvents(void)
{
    for(int i = 0; i < currentLocation->localEventCount; ++i)
    {
        localEvents[i] = ResourceEvent_Load(currentLocation->localEvents[i]);
    }
}

void ResourceLocation_FreeLocalEvents(void)
{
    for(int i = 0; i < MAX_EVENTS; ++i)
    {
        if(localEvents[i])
        {
            ResourceEvent_Free(localEvents[i]);
            localEvents[i] = NULL;
        }
    }
}

uint16_t ResourceStory_GetCurrentAdjacentLocations(void)
{
    if(currentLocation)
    {
        uint16_t count = 0;
        for(int i = 0; i < currentLocation->adjacentLocationCount; ++i)
        {
            if(ResourceEvent_CheckPrerequisites(adjacentLocationInitialEvents[i]))
                ++count;
        }
        return count;
    }
    else
    {
        return 0;
    }
}

const char *ResourceStory_GetAdjacentLocationName(uint16_t index)
{
    uint16_t currentIndex = 0;
    for(int i = 0; i < currentLocation->adjacentLocationCount; ++i)
    {
        if(ResourceEvent_CheckPrerequisites(adjacentLocationInitialEvents[i]))
        {
            if(currentIndex == index)
                return adjacentLocations[i]->name;
            ++currentIndex;
        }
    }
    
    return "None";
}

uint16_t ResourceStory_GetCurrentLocalEvents(void)
{
    if(currentLocation)
    {
        uint16_t count = 0;
        for(int i = 0; i < currentLocation->localEventCount; ++i)
        {
            if(ResourceEvent_CheckPrerequisites(localEvents[i]))
                ++count;
        }
        return count;
    }
    else
    {
        return 0;
    }
}

const char *ResourceStory_GetLocalEventName(uint16_t index)
{
    uint16_t currentIndex = 0;
    for(int i = 0; i < currentLocation->localEventCount; ++i)
    {
        if(ResourceEvent_CheckPrerequisites(localEvents[i]))
        {
            if(currentIndex == index)
                return localEvents[i]->name;
            ++currentIndex;
        }
    }
    
    return "None";
}

int ResourceStory_GetCurrentLocationBackgroundImageId(void)
{
    if(currentLocation && currentLocation->backgroundImageCount)
    {
        uint16_t index = Random(currentLocation->backgroundImageCount);
        return currentLocation->backgroundImages[index];
    }
    else
    {
        return -1;
    }
}

bool ResourceStory_CurrentLocationIsPath(void)
{
    if(currentLocation)
    {
        return currentLocation->length > 0;
    }
    
    return false;
}

bool ResourceStory_CurrentLocationIsRestArea(void)
{
    if(currentLocation)
    {
        return currentLocation->locationProperties & LOCATION_PROPERTY_REST_AREA;
    }
    
    return false;
}

/********************* RESOURCE SKILL ******************************/
void ResourceSkill_Free(Skill *skill)
{
    if(skill)
        free(skill);
}

Skill *ResourceSkill_Load(uint16_t logical_index)
{
    ResHandle currentStoryData = ResourceStory_GetCurrentResHandle();
    Skill *newSkill = calloc(sizeof(Skill), 1);
    ResourceLoadStruct(currentStoryData, logical_index, (uint8_t*)newSkill, sizeof(Skill), "Skill");
    return newSkill;
}

/********************* RESOURCE BATTLER ******************************/

static BattlerWrapper currentMonster = {0};

static BattlerWrapper playerClass = {0};

BattlerWrapper *BattlerWrapper_GetPlayerWrapper(void)
{
    return &playerClass;
}

BattlerWrapper *BattlerWrapper_GetMonsterWrapper(void)
{
    return &currentMonster;
}

Skill *BattlerWrapper_GetSkillByIndex(BattlerWrapper *wrapper, uint16_t index)
{
    return wrapper->loadedSkills[index];
}

uint16_t BattlerWrapper_GetUsableSkillCount(BattlerWrapper *wrapper, uint16_t level)
{
    int count;
    for(count = 0; count < wrapper->battler.skillList.count; ++count)
    {
        if(wrapper->battler.skillList.entries[count].level > level)
            break;
    }
    return count;
}

CombatantClass *BattlerWrapper_GetCombatantClass(BattlerWrapper *wrapper)
{
    return &wrapper->battler.combatantClass;
}

int BattlerWrapper_GetImage(BattlerWrapper *wrapper)
{
    if(wrapper->loaded)
    {
        return wrapper->battler.image;
    }
    else
    {
        return -1;
    }
}

void ResourceBattler_UnloadBattler(BattlerWrapper *wrapper)
{
    if(!wrapper->loaded)
        return;
    
    wrapper->loaded = false;
    for(int i = 0; i < wrapper->battler.skillList.count; ++i)
    {
        ResourceSkill_Free(wrapper->loadedSkills[i]);
        wrapper->loadedSkills[i] = NULL;
    }
    ResourceEvent_Free(wrapper->event);
    wrapper->event = NULL;
}

void ResourceMonster_UnloadCurrent(void)
{
    ResourceBattler_UnloadBattler(&currentMonster);
}

void ResourceBattler_UnloadPlayer(void)
{
    ResourceBattler_UnloadBattler(&playerClass);
}

bool ResourceBattler_LoadBattler(BattlerWrapper *wrapper, uint16_t logical_index)
{
    ResHandle currentStoryData = ResourceStory_GetCurrentResHandle();
    ResourceLoadStruct(currentStoryData, logical_index, (uint8_t*)(&(wrapper->battler)), sizeof(ResourceBattler), "ResourceBattler");
    
    wrapper->battler.image = ImageMap_GetIdByIndex(wrapper->battler.image);
    for(int i = 0; i < wrapper->battler.skillList.count; ++i)
    {
        wrapper->loadedSkills[i] = ResourceSkill_Load(wrapper->battler.skillList.entries[i].id);
        wrapper->battler.skillList.entries[i].id = i;
    }
    
    wrapper->loaded = true;

    if(wrapper->battler.event > 0)
    {
        wrapper->event = ResourceEvent_Load(wrapper->battler.event);
        return ResourceEvent_CheckPrerequisites(wrapper->event);
    }
    
    return true;
}

bool ResourceMonster_LoadCurrent(uint16_t logical_index)
{
    return ResourceBattler_LoadBattler(&currentMonster, logical_index);
}

void ResourceBattler_LoadPlayer(uint16_t classId)
{
    ResourceStory *currentStory = ResourceStory_GetCurrentStory();
    if(!currentStory)
        return;
    
    ResourceBattler_LoadBattler(&playerClass, currentStory->classes[classId]);
}

char *ResourceMonster_GetCurrentName(void)
{
    if(currentMonster.loaded)
    {
        return currentMonster.battler.name;
    }
    else
    {
        return "None";
    }
}

bool ResourceMonster_Loaded(void)
{
    return currentMonster.loaded;
}

bool ResourceStory_CurrentLocationHasMonster(void)
{
    return currentLocation && currentLocation->monsterCount > 0;
}

int ResourceStory_GetCurrentLocationMonster(void)
{
    if(currentLocation && currentLocation->monsterCount)
    {
        uint16_t index = Random(currentLocation->monsterCount);
        return currentLocation->monsters[index];
    }
    else
    {
        return -1;
    }
}

/********************* RESOURCE STORY *******************************/

static ResourceStory **resourceStoryList = NULL;

bool ResourceStory_IsLastResourceStoryIdValid(void)
{
    return isLastResourceStoryIdValid;
}

void ResourceStory_SetLastResourceStoryId(uint16_t id)
{
    lastResourceStoryId = id;
    isLastResourceStoryIdValid = true;
}

uint16_t ResourceStory_GetLastResourceStoryId(void)
{
    return lastResourceStoryId;
}

static ResourceStory *ResourceStory_GetCurrentStory(void)
{
    if(currentResourceStoryIndex < 0)
        return NULL;
    
    return resourceStoryList[currentResourceStoryIndex];
}

int16_t ResourceStory_GetStoryIndexById(uint16_t id)
{
    for(int i = 0; i < GetStoryCount(); ++i)
    {
        if(resourceStoryList[i]->id == id)
            return i;
    }
    return -1;
}

static ResHandle ResourceStory_GetCurrentResHandle(void)
{
    if(currentResourceStoryIndex < 0)
        return NULL;
    
    return resource_get_handle(GetStoryResourceIdByIndex(currentResourceStoryIndex));
}

void ResourceStory_InitializeCurrent(void)
{
    currentResourceStoryState.persistedResourceStoryState.currentLocationIndex = ResourceStory_GetCurrentStory()->start_location;
    currentResourceStoryState.persistedResourceStoryState.timeOnPath = 0;
    for(int i = 0; i < MAX_GAME_STATE_VARIABLES; ++i)
    {
        currentResourceStoryState.persistedResourceStoryState.gameState[i] = 0;
    }
    
    ResourceStory_UpdateStoryWithPersistedState();
}

uint16_t ResourceStory_GetCurrentLocationIndex(void)
{
    return currentResourceStoryState.persistedResourceStoryState.currentLocationIndex;
}

bool ResourceStory_IncrementTimeOnPath(void)
{
    currentResourceStoryState.persistedResourceStoryState.timeOnPath++;
    return currentResourceStoryState.persistedResourceStoryState.timeOnPath >= currentLocation->length;
}

uint16_t ResourceStory_GetTimeOnPath(void)
{
    return currentResourceStoryState.persistedResourceStoryState.timeOnPath;
}

uint16_t ResourceStory_GetCurrentLocationLength(void)
{
    if(currentLocation)
    {
        return currentLocation->length;
    }
    
    return 0;
}

const char *ResourceStory_GetCurrentLocationName(void)
{
    if(currentLocation)
    {
        return currentLocation->name;
    }
    else
    {
        return "None";
    }
}

ResourceStoryUpdateReturnType ResourceStory_UpdateCurrentLocation(void)
{
    if(ResourceStory_CurrentLocationIsPath())
    {
        bool pathEnded = ResourceStory_IncrementTimeOnPath();
        if(pathEnded)
        {
            return ResourceStory_MoveToLocation(currentResourceStoryState.persistedResourceStoryState.destinationIndex);
        }
        else
        {
            return STORYUPDATE_COMPUTERANDOM;
        }
    }

    return STORYUPDATE_DONOTHING;
}

ResourceStoryUpdateReturnType ResourceStory_MoveToLocation(uint16_t index)
{
    uint16_t newIndex = 0;
    uint16_t indexToUse = 0;
    for(int i = 0; i < currentLocation->adjacentLocationCount; ++i)
    {
        if(ResourceEvent_CheckPrerequisites(adjacentLocationInitialEvents[i]))
        {
            if(newIndex == index)
            {
                indexToUse = i;
                break;
            }
            ++newIndex;
        }
    }

    if(currentLocation)
    {
        ResourceLocation *newLocation = adjacentLocations[indexToUse];
        ResourceEvent *newInitialEvent = adjacentLocationInitialEvents[indexToUse];
        adjacentLocations[indexToUse] = NULL;
        adjacentLocationInitialEvents[indexToUse] = NULL;

        uint16_t globalIndex = currentLocation->adjacentLocations[indexToUse];
        uint16_t oldIndex = currentResourceStoryState.persistedResourceStoryState.currentLocationIndex;
        
        ResourceLocation_FreeLocalEvents();
        ResourceLocation_FreeAdjacentLocations();
        ResourceLocation_Free(currentLocation);
        ResourceEvent_Free(currentLocationInitialEvent);

        currentLocation = newLocation;
        currentLocationInitialEvent = newInitialEvent;
        if(currentLocationInitialEvent)
            ResourceEvent_TriggerEvent(currentLocationInitialEvent, true);
        ResourceLocation_LoadAdjacentLocations();
        ResourceLocation_LoadLocalEvents();
        currentResourceStoryState.persistedResourceStoryState.currentLocationIndex = globalIndex;
        currentResourceStoryState.persistedResourceStoryState.timeOnPath = 0;
        currentResourceStoryState.persistedResourceStoryState.encounterChance = currentLocation->encounterChance;
        currentResourceStoryState.persistedResourceStoryState.pathLength = currentLocation->length;

        if(currentLocation && (currentLocation->locationProperties & LOCATION_PROPERTY_REST_AREA))
        {
            Character_Rest();
        }
        
        if(currentLocation && (currentLocation->locationProperties & LOCATION_PROPERTY_LEVEL_UP))
        {
            Character_GrantLevel();
        }

        if(currentLocation && (currentLocation->locationProperties & LOCATION_PROPERTY_GAME_WIN))
        {
            return STORYUPDATE_WIN;
        }

        if(ResourceStory_CurrentLocationIsPath())
        {
            if(currentLocation->adjacentLocations[0] == oldIndex)
                currentResourceStoryState.persistedResourceStoryState.destinationIndex = 1;
            else
                currentResourceStoryState.persistedResourceStoryState.destinationIndex = 0;
        }
        else
        {
            if(ResourceStory_CurrentLocationHasMonster())
            {
                return STORYUPDATE_TRIGGER_BATTLE;
            }
        }
        return STORYUPDATE_FULLREFRESH;
    }
    return STORYUPDATE_DONOTHING;
}

void ResourceStory_FreeAll(void)
{
    for(int i = 0; i < GetStoryCount(); ++i)
    {
        free(resourceStoryList[i]);
    }
    free(resourceStoryList);
}

static ResourceStory *ResourceStory_Load(int resourceId)
{
    ResHandle currentStoryData = resource_get_handle(resourceId);
    ResourceStory *currentResourceStory = calloc(sizeof(ResourceStory), 1);
    ResourceLoadStruct(currentStoryData, 0, (uint8_t*)currentResourceStory, sizeof(ResourceStory), "ResourceStory");
    return currentResourceStory;
}

#if DEBUG_LOGGING > 1
static void ResourceStory_Log(ResourceStory *story)
{
    DEBUG_VERBOSE_LOG("ResourceStory: %s, %s, %d, %d, %d", story->name, story->description, story->id, story->version, story->start_location);
}
#endif

void ResourceStory_LoadAll(void)
{
    if(resourceStoryList)
        return;
    
    resourceStoryList = calloc(sizeof(ResourceStory*), GetStoryCount());
    
    for(int i = 0; i < GetStoryCount(); ++i)
    {
        resourceStoryList[i] = ResourceStory_Load(GetStoryResourceIdByIndex(i));
#if DEBUG_LOGGING > 1
        ResourceStory_Log(resourceStoryList[i]);
#endif
    }
}

void ResourceStory_SetCurrentStory(uint16_t index)
{
    currentResourceStoryIndex = index;
    ResourceStory_SetLastResourceStoryId(resourceStoryList[currentResourceStoryIndex]->id);
}

void ResourceStory_ClearCurrentStory(void)
{
    currentResourceStoryIndex = -1;
    if(currentLocation)
    {
        ResourceLocation_Free(currentLocation);
        currentLocation = NULL;
        ResourceEvent_Free(currentLocationInitialEvent);
        currentLocationInitialEvent = NULL;
        ResourceLocation_FreeAdjacentLocations();
        ResourceLocation_FreeLocalEvents();
    }
}

void ResourceStory_LogCurrent(void)
{
#if DEBUG_LOGGING
    ResourceStory *currentResourceStory = ResourceStory_GetCurrentStory();
    DEBUG_LOG("ResourceStory: %s, %s, %d, %d, %d", currentResourceStory->name, currentResourceStory->description, currentResourceStory->id, currentResourceStory->version, currentResourceStory->start_location);
#endif
}

const char *ResourceStory_GetNameByIndex(uint16_t index)
{
    if(index >= GetStoryCount())
        return "None";
        
    ResourceStory *story = resourceStoryList[index];
    return story->name;
}

const char *ResourceStory_GetDescriptionByIndex(uint16_t index)
{
    if(index >= GetStoryCount())
        return "None";
    
    ResourceStory *story = resourceStoryList[index];
    return story->description;
}

bool ResourceStory_InStory(void)
{
    return currentResourceStoryIndex != -1;
}

void ResourceStory_GetStoryList(uint16_t *count, uint16_t **buffer)
{
    *count = GetStoryCount();
    *buffer = calloc(sizeof(uint16_t), *count);
    
    for(int i = 0; i < *count; ++i)
    {
        (*buffer)[i] = resourceStoryList[i]->id;
    }
}

uint16_t ResourceStory_GetCurrentStoryId(void)
{
    return ResourceStory_GetCurrentStory()->id;
}

uint16_t ResourceStory_GetCurrentStoryVersion(void)
{
    return ResourceStory_GetCurrentStory()->version;
}

uint16_t ResourceStory_GetCurrentStoryHash(void)
{
    return ResourceStory_GetCurrentStory()->hash;
}

uint16_t ResourceStory_GetCurrentStoryXPMonstersPerLevel(void)
{
    return ResourceStory_GetCurrentStory()->xpMonstersPerLevel;
}

uint16_t ResourceStory_GetCurrentStoryXPDifferenceScale(void)
{
    return ResourceStory_GetCurrentStory()->xpDifferenceScale;
}

void ResourceStory_GetPersistedData(uint16_t *count, uint8_t **buffer)
{
    *count = sizeof(currentResourceStoryState.persistedResourceStoryState);
    *buffer = (uint8_t*)(&currentResourceStoryState.persistedResourceStoryState);
}

void ResourceStory_UpdateStoryWithPersistedState(void)
{
    if(currentLocation)
    {
        ResourceLocation_Free(currentLocation);
        ResourceEvent_Free(currentLocationInitialEvent);
    }
    
    currentLocation = ResourceLocation_Load(currentResourceStoryState.persistedResourceStoryState.currentLocationIndex);
    currentLocationInitialEvent = ResourceEvent_Load(currentLocation->initialEvent);
    currentResourceStoryState.persistedResourceStoryState.encounterChance = currentLocation->encounterChance;
    currentResourceStoryState.persistedResourceStoryState.pathLength = currentLocation->length;
    ResourceBattler_UnloadPlayer();
    ResourceLocation_FreeAdjacentLocations();
    ResourceLocation_LoadAdjacentLocations();
    ResourceLocation_FreeLocalEvents();
    ResourceLocation_LoadLocalEvents();
#if DEBUG_LOGGING > 1
    ResourceLocation_Log(currentLocation);
#endif
}
