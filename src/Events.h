//
//  Events.h
//  
//
//  Created by Jonathan Panttaja on 1/25/16.
//
//

#pragma once

typedef struct Event Event;

Event *Event_Load(uint16_t logical_index);
void Event_Free(Event *event);
void Event_LoadLocalEvents(uint16_t count, uint16_t *eventIds);
void Event_FreeLocalEvents(void);

uint16_t Event_GetCurrentLocalEvents(void);
const char *Event_GetLocalEventName(uint16_t index);
const char *Event_GetLocalEventDescription(uint16_t index);

bool Event_CheckPrerequisites(Event *event);
void Event_Trigger(uint16_t index);
void Event_TriggerEvent(Event *event, bool now);

void Event_UpdateGameState_Push(void *data);
