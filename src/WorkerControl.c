#include <pebble.h>
#include "Adventure.h"
#include "Logging.h"
#include "OptionsMenu.h"
#include "WorkerControl.h"
#include "Utils.h"

void SendMessageToWorker(uint8_t type, uint16_t data0, uint16_t data1, uint16_t data2)
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

void AppDying(void)
{
#if ALLOW_WORKER_APP
	if(WorkerIsRunning())
		SendMessageToWorker(APP_DYING, 0, 0, 0);
#endif
}

void AppAwake(void)
{
#if ALLOW_WORKER_APP
	INFO_LOG("AppAwake");
	if(WorkerIsRunning())
		SendMessageToWorker(APP_AWAKE, 0, 0, 0);
#endif
}

AppWorkerResult LaunchWorkerApp()
{
#if ALLOW_WORKER_APP
	INFO_LOG("Launching worker app");
	return app_worker_launch();
#else
	return APP_WORKER_RESULT_NO_WORKER;
#endif
}

AppWorkerResult KillWorkerApp()
{
#if ALLOW_WORKER_APP
	bool running = WorkerIsRunning();
	
	if(!running)
		return false;
	
	INFO_LOG("Killing worker app");
	return app_worker_kill();
#else
	return APP_WORKER_RESULT_NO_WORKER;
#endif
}

void AttemptToLaunchWorkerApp()
{
#if ALLOW_WORKER_APP
	if(WorkerIsRunning())
	{
		INFO_LOG("WorkerApp already running");
		SetWorkerApp(true);
	}
	else
	{
#if ALLOW_WORKER_APP_LISTENING
		app_worker_message_subscribe(WorkerMessageHandler);
#endif
        
		LaunchWorkerApp();
		SetWorkerApp(false);
	}
#endif
}

void AttemptToKillWorkerApp()
{
#if ALLOW_WORKER_APP
	if(WorkerIsRunning())
	{
		KillWorkerApp();
	}
	else
	{
		SetWorkerApp(false);
	}
#endif	
}

bool WorkerIsRunning(void)
{
#if ALLOW_WORKER_APP
	return app_worker_is_running();
#else
	return false;
#endif
}

void WorkerMessageHandler(uint16_t type, AppWorkerMessage *data)
{
#if ALLOW_WORKER_APP
	DEBUG_VERBOSE_LOG("Worker message handler");
	switch(type)
	{
		case WORKER_LAUNCHED:
		{
			DEBUG_LOG("Worker Launched.");
			SetWorkerApp(true);
			AppAwake();
			break;
		}
		case WORKER_DYING:
		{
			DEBUG_LOG("Worker dying");
			SetWorkerApp(false);
			break;
		}
		default:
		{
			DEBUG_VERBOSE_LOG("Undefined worker message: %d", type);
		}
	}
#endif
}
