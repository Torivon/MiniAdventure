#pragma once

AppWorkerResult LaunchWorkerApp();
AppWorkerResult KillWorkerApp();
void AttemptToKillWorkerApp();
void AttemptToLaunchWorkerApp();

bool WorkerIsRunning(void);
void WorkerMessageHandler(uint16_t type, AppWorkerMessage *data);
void AppDying(void);
void AppAwake(void);
void SendWorkerCanLaunch(void);

// Worker message types
enum
{
	WORKER_LAUNCHED = 0,
	WORKER_DYING,

	APP_DYING,
	APP_AWAKE,
};