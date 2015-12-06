#include "pebble.h"

#include "Adventure.h"
#include "Logging.h"
#include "Menu.h"
#include "UILayers.h"
#include "Utils.h"

// Called once per minute
void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) 
{
	DEBUG_LOG("Main App tick");	
	UpdateClock(tick_time);
	UpdateAdventure();
	if(units_changed & DAY_UNIT)
	{
		UpdateDay(tick_time);
	}
	if(units_changed & MONTH_UNIT)
	{
		UpdateMonth(tick_time);
	}
	if(units_changed & YEAR_UNIT)
	{
		UpdateYear(tick_time);
	}
}

void handle_init() {
	
	INFO_LOG("Starting MiniDungeon");
	time_t now = time(NULL);
	
	DEBUG_LOG("Srand");
	srand(now);
		
	ShowAdventureWindow();
	struct tm *current_time = localtime(&now);
	handle_minute_tick(current_time, MINUTE_UNIT | DAY_UNIT | MONTH_UNIT | YEAR_UNIT);
	tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
}

void handle_deinit() 
{
	INFO_LOG("Cleaning up on exit.");
	UnloadBackgroundImage();
	UnloadMainBmpImage();
	UnloadTextLayers();
	tick_timer_service_unsubscribe();
}

// The main event/run loop for our app
int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}