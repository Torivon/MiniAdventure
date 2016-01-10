#include <pebble.h>
#include "Adventure.h"
#include "Battle.h"
#include "DialogFrame.h"
#include "GlobalState.h"
#include "LargeImage.h"
#include "Logging.h"
#include "Menu.h"
#include "OptionsMenu.h"
#include "TitleScreen.h"

typedef struct GlobalStateInstance GlobalStateInstance;

typedef struct GlobalStateInstance
{
	GlobalState state;
	TimeUnits triggerUnits;
	void *data;
    GlobalStateInstance *prev;
    GlobalStateInstance *next;
} GlobalStateInstance;

static GlobalStateInstance *stackTop = NULL;
static GlobalStateInstance *queueFront = NULL;
static GlobalStateInstance *queueBack = NULL;

static void PopAppear(void *data)
{
    GlobalState_Pop();
    GlobalState_Pop();
}

void GlobalState_RunPushCallback(GlobalStateInstance *instance)
{
    switch(instance->state)
    {
        case STATE_OPTIONS:
        {
            OptionScreenPush(instance->data);
            break;
        }
        case STATE_MENU:
        {
            ShowMenu(instance->data);
            break;
        }
        case STATE_LARGE_IMAGE:
        {
            LargeImagePush(instance->data);
            break;
        }
        case STATE_BATTLE:
        {
            BattleScreenPush(instance->data);
            break;
        }
        case STATE_RESET_GAME:
        {
            ResetGamePush(instance->data);
            break;
        }
        case STATE_ADVENTURE:
        {
            AdventureScreenPush(instance->data);
            break;
        }
        default:
        {
            break;
        }
    }
}

void GlobalState_RunAppearCallback(GlobalStateInstance *instance)
{
    switch(instance->state)
    {
        case STATE_OPTIONS:
        {
            OptionScreenAppear(instance->data);
            break;
        }
        case STATE_STATE_POP:
        {
            PopAppear(instance->data);
            break;
        }
        case STATE_DIALOG:
        {
            DialogAppear(instance->data);
            break;
        }
        case STATE_BATTLE:
        {
            BattleScreenAppear(instance->data);
            break;
        }
        case STATE_ADVENTURE:
        {
            AdventureScreenAppear(instance->data);
            break;
        }
        case STATE_TITLE_SCREEN:
        {
            TitleScreen_Appear(instance->data);
            break;
        }
        default:
        {
            break;
        }
    }
}

void GlobalState_RunUpdateCallback(GlobalStateInstance *instance, TimeUnits units_changed)
{
    if(units_changed & instance->triggerUnits)
    {
        switch(instance->state)
        {
            case STATE_BATTLE:
            {
                UpdateBattle(instance->data);
                break;
            }
            case STATE_ADVENTURE:
            {
                UpdateAdventure(instance->data);
                break;
            }
            default:
            {
                break;
            }
        }
    }
}

void GlobalState_RunDisappearCallback(GlobalStateInstance *instance)
{
    switch(instance->state)
    {
        case STATE_DIALOG:
        {
            DialogDisappear(instance->data);
            break;
        }
        case STATE_ADVENTURE:
        {
            AdventureScreenDisappear(instance->data);
            break;
        }
        default:
        {
            break;
        }
    }
}

void GlobalState_RunPopCallback(GlobalStateInstance *instance)
{
    switch(instance->state)
    {
        case STATE_OPTIONS:
        {
            OptionScreenPop(instance->data);
            break;
        }
        case STATE_LARGE_IMAGE:
        {
            LargeImagePop(instance->data);
            break;
        }
        case STATE_BATTLE:
        {
            BattleScreenPop(instance->data);
            break;
        }
        case STATE_RESET_GAME:
        {
            ResetGamePop(instance->data);
            break;
        }
        case STATE_ADVENTURE:
        {
            AdventureScreenPop(instance->data);
            break;
        }
        case STATE_TITLE_SCREEN:
        {
            TitleScreen_Pop(instance->data);
            break;
        }
        default:
        {
            break;
        }
    }
}

void GlobalState_Queue(GlobalState state,
                      TimeUnits triggerUnits,
                      void *data)
{
    GlobalStateInstance *newInstance = calloc(sizeof(GlobalStateInstance), 1);
    
    newInstance->state = state;
    newInstance->triggerUnits = triggerUnits;
    newInstance->data = data;
    newInstance->next = NULL;
    newInstance->prev = NULL;
    
    if(queueBack == NULL)
    {
        queueFront = queueBack = newInstance;
    }
    else
    {
        queueBack->next = newInstance;
        newInstance->prev = queueBack;
        queueBack = newInstance;
    }
}

static void GlobalState_PushInternal(GlobalStateInstance *newInstance)
{
    newInstance->next = NULL;
    newInstance->prev = NULL;
    
    if(stackTop == NULL)
    {
        stackTop = newInstance;
    }
    else
    {
        stackTop->next = newInstance;
        newInstance->prev = stackTop;
        stackTop = newInstance;
    }
    
    GlobalState_RunPushCallback(newInstance);
    GlobalState_RunAppearCallback(newInstance);
}

GlobalStateInstance *GlobalState_PopQueue(void)
{
    if(!queueBack)
        return NULL;
    
    GlobalStateInstance *instance = queueFront;
    queueFront = instance->next;
    if(queueFront)
    {
        queueFront->prev = NULL;
    }
    else
    {
        queueBack = NULL;
    }
    return instance;
}

void GlobalState_ClearQueue(void)
{
    while(queueBack)
    {
        GlobalStateInstance *instance = GlobalState_PopQueue();
        free(instance);
    }
}

void GlobalState_QueueStatePop(void)
{
    GlobalState_Queue(STATE_STATE_POP, 0, NULL);
}

void GlobalState_Push(GlobalState state,
                                     TimeUnits triggerUnits,
                                     void *data)
{
    DEBUG_LOG("Push global state: %d", state);
    // Pushing a new instance clears the queue
    
    GlobalState_ClearQueue();
    
    if(stackTop)
    {
        GlobalState_RunDisappearCallback(stackTop);
    }
    
    GlobalStateInstance *newInstance = calloc(sizeof(GlobalStateInstance), 1);
    
    newInstance->state = state;
    newInstance->triggerUnits = triggerUnits;
    newInstance->data = data;

    GlobalState_PushInternal(newInstance);
}

void GlobalState_Update(TimeUnits units_changed)
{
	if(!stackTop)
		return;

    GlobalState_RunUpdateCallback(stackTop, units_changed);
}

void GlobalState_Pop(void)
{
    if(!stackTop)
        return;
	
    GlobalStateInstance *oldInstance = stackTop;
    
	DEBUG_LOG("pop global state: %d", oldInstance->state);
    GlobalState_RunDisappearCallback(oldInstance);
    GlobalState_RunPopCallback(oldInstance);
	
    stackTop = oldInstance->prev;
    if(stackTop)
        stackTop->next = NULL;
    
    free(oldInstance);
    
    if(queueBack)
    {
        GlobalStateInstance *queuedInstance = GlobalState_PopQueue();
        if(queuedInstance)
        {
            GlobalState_PushInternal(queuedInstance);
            return;
        }
    }
    
	if(!stackTop)
	{
        window_stack_pop(true);
		return;
	}

    GlobalState_RunAppearCallback(stackTop);
}

GlobalState GlobalState_GetCurrent(void)
{
    if(!stackTop)
        return STATE_NONE;
	
	return stackTop->state;
}

void GlobalState_PopAll(void)
{
	while(stackTop)
	{
		GlobalState_Pop();
	}
}

void GlobalState_Initialize(void)
{
}

void GlobalState_Free(void)
{
    GlobalState_PopAll();
}
