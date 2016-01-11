#include "../src/MiniAdventure.h"
#ifdef BUILD_WORKER_FILES
#include <pebble_worker.h>

#include "../src/Utils.h"
#include "../src/WorkerControl.h"
#include "Worker_Persistence.h"
#include "../src/ResourceStory.h"

#if ALLOW_WORKER_APP

void SendMessageToApp(uint8_t type, uint16_t data0, uint16_t data1, uint16_t data2)
{
#if ALLOW_WORKER_APP_MESSAGES
	AppWorkerMessage msg_data = {
		.data0 = data0,
		.data1 = data1,
		.data2 = data2
	};
	app_worker_send_message(type, &msg_data);
#endif
}

static bool handlingTicks = false;
static bool forcedDelay = false; // Make sure we don't trigger an event while the app is still closing
static bool appAlive = true; 
static bool error = false;

void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) 
{	
	if(appAlive)
		return;
	
    if(GetClosedInBattle())
		return;
    
    if(!IsCurrentStoryValid())
        return;

	if(handlingTicks)
	{
		if(forcedDelay)
		{
			forcedDelay = false;
			return;
		}
        
        PersistedResourceStoryState *storyState = GetPersistedStoryState();
        
        if(storyState->pathLength > 0)
        {
            storyState->timeOnPath++;
            bool launch = false;
            if(storyState->timeOnPath >= storyState->pathLength)
            {
                launch = true;
                handlingTicks = false;
            }
            else
            {
                int roll = Random_inline(100) + 1;

                if(roll <= storyState->encounterChance)
                {
                    ForceRandomBattle();
                    launch = true;
                    handlingTicks = false;
                }
            }
            SaveWorkerData();
            if(launch && GetWorkerCanLaunch())
                worker_launch_app();
        }
    }
}

static void InitializeState(void)
{
	handlingTicks = !GetClosedInBattle(); // Don't handle ticks while in combat
	forcedDelay = true;
	appAlive = false;
	error = false;	
}

static void AppMessageHandler(uint16_t type, AppWorkerMessage *data)
{
	switch(type)
	{
		case APP_DYING:
		{
            LoadWorkerData();
			InitializeState();
			break;
		}
		case APP_AWAKE:
		{
			handlingTicks = false;
			appAlive = true;
			break;
		}
	}
}

static void init() 
{
	// Initialize your worker here
	time_t now = time(NULL);
	srand(now);
	if(!LoadWorkerData())
		worker_launch_app();
#if ALLOW_WORKER_APP_LISTENING
	app_worker_message_subscribe(AppMessageHandler);
#endif
	InitializeState();
	SendMessageToApp(WORKER_LAUNCHED, 0, 0, 0);
	tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
}

static void deinit() {
	// Deinitialize your worker here
	SendMessageToApp(WORKER_DYING, 0, 0, 0);
#if ALLOW_WORKER_APP_LISTENING
	app_worker_message_unsubscribe();
#endif
	tick_timer_service_unsubscribe();
}


int main(void) {
	init();
	worker_event_loop();
	deinit();
}
#else

int main(void)
{
    worker_event_loop();
}

#endif
#endif