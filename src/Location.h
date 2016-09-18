//
//  Location.h
//  
//
//  Created by Jonathan Panttaja on 1/25/16.
//
//
#pragma once

typedef struct Location Location;

bool Location_CurrentLocationIsPath(void);
void Location_LoadAdjacentLocations(void);
void Location_FreeAdjacentLocations(void);
void Location_SetCurrentLocation(uint16_t locationIndex);
void Location_ClearCurrentLocation(void);

uint16_t Location_GetCurrentAdjacentLocations(void);
uint16_t Location_GetCurrentAdjacentLocationByIndex(uint16_t index);
const char *Location_GetAdjacentLocationName(uint16_t index);
const char *Location_GetAdjacentLocationDescription(uint16_t index);

uint16_t Location_GetCurrentLength(void);
const char *Location_GetCurrentName(void);
int Location_GetCurrentBackgroundImageId(void);
int Location_GetCurrentBattleFloorImageId(void);
uint16_t Location_GetCurrentEncounterChance(void);
uint16_t Location_GetCurrentBaseLevel(void);
bool Location_CurrentLocationHasMonster(void);
int Location_GetCurrentMonster(void);
bool Location_CurrentLocationIsRestArea(void);
bool Location_CurrentLocationIsLevelUp(void);
bool Location_CurrentLocationEndsGame(void);

uint16_t Location_MoveToAdjacentLocation(uint16_t menuIndex);

bool Location_CurrentLocationUseActivityTracking(void);
uint16_t Location_CurrentInactiveSpeed(void);
uint16_t Location_CurrentActiveSpeed(void);
bool Location_CurrentSkipEncountersIfActive(void);
bool Location_CurrentGrantXPForSkippedEncounters(void);
bool Location_CurrentExtendPathDuringActivity(void);
