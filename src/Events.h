//
//  Events.h
//  
//
//  Created by Jonathan Panttaja on 1/25/16.
//
//

#pragma once

#include "AutoSizeConstants.h"

typedef struct EventStateChange
{
    uint16_t positiveStateChanges[MAX_GAME_STATE_VARIABLES];
    uint16_t negativeStateChanges[MAX_GAME_STATE_VARIABLES];
} EventStateChange;

typedef struct Event Event;
typedef struct KeyItem KeyItem;

Event *Event_Load(uint16_t logical_index);
void Event_Free(Event *event);
void Event_LoadLocalEvents(uint16_t count, uint16_t *eventIds);
void Event_FreeLocalEvents(void);

KeyItem *KeyItem_Load(uint16_t logical_index);
void KeyItem_Free(KeyItem *keyItem);
uint16_t KeyItem_GetGameStateIndex(KeyItem *keyItem);
char *KeyItem_GetName(KeyItem *keyItem);

uint16_t Event_GetCurrentLocalEvents(void);
const char *Event_GetLocalEventName(uint16_t index);
const char *Event_GetLocalEventDescription(uint16_t index);

bool Event_CheckPrerequisites(Event *event);
bool EventList_CheckPrerequisites(Event **eventList, uint16_t *index);
void Event_Trigger(uint16_t index);
void Event_TriggerEvent(Event *event, bool now);

void Event_UpdateGameState_Push(void *data);
