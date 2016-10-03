//
//  Events.c
//  
//
//  Created by Jonathan Panttaja on 1/25/16.
//
//

#include <pebble.h>
#include "BinaryResourceLoading.h"
#include "DialogFrame.h"
#include "Events.h"
#include "GlobalState.h"
#include "logging.h"
#include "Story.h"

typedef struct Event
{
    char name[MAX_STORY_NAME_LENGTH];
    char menuDescription[MAX_STORY_DESC_LENGTH];
    uint16_t dialog;
    uint16_t usePrerequisites;
    uint16_t positivePrerequisites[MAX_GAME_STATE_VARIABLES];
    uint16_t negativePrerequisites[MAX_GAME_STATE_VARIABLES];
    EventStateChange stateChanges;
} Event;

static uint16_t localEventCount = 0;
static Event *localEvents[MAX_EVENTS] = {0};

Event *Event_Load(uint16_t logical_index)
{
    if(logical_index == 0)
        return NULL;
    
    Event *event = calloc(sizeof(Event), 1);
    ResourceLoadStruct(Story_GetCurrentResHandle(), logical_index, (uint8_t*)event, sizeof(Event), "Event");
    return event;
}

void Event_Free(Event *event)
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
            if(!positive)
                gameState = ~gameState;
            bool result = ((gameState & prerequisite) == prerequisite);
            match = match && result;
        }
    }
    return match;
}

bool Event_CheckPrerequisites(Event *event)
{
    if(!event || !event->usePrerequisites)
        return true;
    
    uint16_t *gameStateList = Story_GetCurrentGameState();
    
    bool match = true;
    match = match && CheckPrerequisites(gameStateList, event->positivePrerequisites, true);
    match = match && CheckPrerequisites(gameStateList, event->negativePrerequisites, false);
    return match;
}

bool EventList_CheckPrerequisites(Event **eventList, uint16_t *index)
{
    // If there are no events, return true, if there are some non-zero events only return true
    // if one of the events is true.
    if(!eventList || !eventList[0])
    {
        if(index)
            *index = 0;
        return true;
    }
    
    for(int i = 0; i < MAX_EVENTS; ++i)
    {
        Event *event = eventList[i];
        if(!event)
            return false;
        
        if(Event_CheckPrerequisites(event))
        {
            if(index)
                *index = i;
            return true;
        }
    }
    return false;
}

void Event_UpdateGameState(void *data)
{
    Story_UpdateGameState((EventStateChange *)data);
}

void Event_UpdateGameState_Push(void *data)
{
    Event *event = (Event*)data;
    Event_UpdateGameState(event);
    GlobalState_Pop();
}

void Event_TriggerEvent(Event *event, bool now)
{
    if(event->dialog > 0)
    {
        if(now)
            Dialog_TriggerFromResource(Story_GetCurrentResHandle(), event->dialog);
        else
            Dialog_QueueFromResource(Story_GetCurrentResHandle(), event->dialog);
        GlobalState_Queue(STATE_UPDATE_GAME_STATE, 0, &event->stateChanges);
    }
    else
    {
        if(now)
            GlobalState_Push(STATE_UPDATE_GAME_STATE, 0, &event->stateChanges);
        else
            GlobalState_Queue(STATE_UPDATE_GAME_STATE, 0, &event->stateChanges);
    }
}

void Event_Trigger(uint16_t index)
{
    uint16_t newIndex = 0;
    uint16_t indexToUse = 0;
    for(int i = 0; i < localEventCount; ++i)
    {
        if(Event_CheckPrerequisites(localEvents[i]))
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
        Dialog_TriggerFromResource(Story_GetCurrentResHandle(), localEvents[indexToUse]->dialog);
        GlobalState_Queue(STATE_UPDATE_GAME_STATE, 0, &localEvents[indexToUse]->stateChanges);
    }
    else
    {
        GlobalState_Push(STATE_UPDATE_GAME_STATE, 0, &localEvents[indexToUse]->stateChanges);
    }
}

void Event_Queue(uint16_t index)
{
    uint16_t newIndex = 0;
    uint16_t indexToUse = 0;
    for(int i = 0; i < localEventCount; ++i)
    {
        if(Event_CheckPrerequisites(localEvents[i]))
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
        Dialog_QueueFromResource(Story_GetCurrentResHandle(), localEvents[indexToUse]->dialog);
    GlobalState_Queue(STATE_UPDATE_GAME_STATE, 0, &localEvents[indexToUse]->stateChanges);
}


void Event_LoadLocalEvents(uint16_t count, uint16_t *eventIds)
{
    localEventCount = count;
    for(int i = 0; i < localEventCount; ++i)
    {
        localEvents[i] = Event_Load(eventIds[i]);
    }
}

void Event_FreeLocalEvents(void)
{
    for(int i = 0; i < MAX_EVENTS; ++i)
    {
        if(localEvents[i])
        {
            Event_Free(localEvents[i]);
            localEvents[i] = NULL;
        }
    }
}

uint16_t Event_GetCurrentLocalEvents(void)
{
    if(localEventCount > 0)
    {
        uint16_t count = 0;
        for(int i = 0; i < localEventCount; ++i)
        {
            if(Event_CheckPrerequisites(localEvents[i]))
                ++count;
        }
        return count;
    }
    else
    {
        return 0;
    }
}

const char *Event_GetLocalEventName(uint16_t index)
{
    uint16_t currentIndex = 0;
    for(int i = 0; i < localEventCount; ++i)
    {
        if(Event_CheckPrerequisites(localEvents[i]))
        {
            if(currentIndex == index)
                return localEvents[i]->name;
            ++currentIndex;
        }
    }
    
    return "None";
}

const char *Event_GetLocalEventDescription(uint16_t index)
{
    uint16_t currentIndex = 0;
    for(int i = 0; i < localEventCount; ++i)
    {
        if(Event_CheckPrerequisites(localEvents[i]))
        {
            if(currentIndex == index)
                return localEvents[i]->menuDescription;
            ++currentIndex;
        }
    }
    
    return "None";
}

