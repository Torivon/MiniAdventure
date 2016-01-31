#include <pebble.h>

#include "Adventure.h"
#include "BinaryResourceLoading.h"
#include "Character.h"
#include "DescriptionFrame.h"
#include "DialogFrame.h"
#include "EngineInfo.h"
#include "EngineMenu.h"
#include "GlobalState.h"
#include "ImageMap.h"
#include "LargeImage.h"
#include "Logging.h"
#include "MainImage.h"
#include "Menu.h"
#include "MiniAdventure.h"
#include "OptionsMenu.h"
#include "BaseWindow.h"
#include "Story.h"
#include "StoryList.h"

static bool tutorialSeen = false;
static bool firstLaunch = true;

void SetTutorialSeen(bool enable)
{
    tutorialSeen = enable;
}

bool GetTutorialSeen(void)
{
    return tutorialSeen;
}

void LaunchStory(uint16_t index, bool now)
{
    Story_SetCurrentStory(index);
    if(now)
        Adventure_Trigger();
    else
        Adventure_Queue();
}

uint16_t TitleScreen_MenuSectionCount(void)
{
    return 1 + EngineMenu_GetSectionCount();
}

const char *TitleScreen_MenuSectionName(uint16_t sectionIndex)
{
    switch(sectionIndex)
    {
        case 0:
            return "Stories";
        case 1:
            return EngineMenu_GetSectionName();
    }
    return "None";
}

uint16_t TitleScreen_MenuCellCount(uint16_t sectionIndex)
{
    switch(sectionIndex)
    {
        case 0:
            return GetStoryCount();
        case 1:
            return EngineMenu_GetCellCount();
    }
    return 0;
}

const char *TitleScreen_MenuCellName(MenuIndex *index)
{
    switch(index->section)
    {
        case 0:
            return Story_GetNameByIndex(index->row);
        case 1:
            return EngineMenu_GetCellName(index->row);
    }
    return "None";
}

const char *TitleScreen_MenuCellDescription(MenuIndex *index)
{
    switch(index->section)
    {
        case 0:
            return Story_GetDescriptionByIndex(index->row);
        case 1:
            return EngineMenu_GetCellName(index->row);
    }
    return "None";
}

void TitleScreen_MenuSelect(MenuIndex *index)
{
    switch(index->section)
    {
        case 0:
            return LaunchStory(index->row, false);
        case 1:
            return EngineMenu_SelectAction(index->row);
    }
}

void TitleScreen_Appear(void *data)
{
    RegisterMenuState(GetMainMenu(), STATE_TITLE_SCREEN);
    RegisterMenuState(GetSlaveMenu(), STATE_NONE);
	SetForegroundImage(ImageMap_GetIdByIndex(EngineInfo_GetInfo()->titleImage));
	SetMainImageVisibility(true, true, false);
	SetDescription("MiniAdventure");
    if(launch_reason() == APP_LAUNCH_WORKER && Story_IsLastStoryIdValid() && firstLaunch)
    {
        LaunchStory(Story_GetStoryIndexById(Story_GetLastStoryId()), true);
        firstLaunch = false;
    }
}

void TitleScreen_Pop(void *data)
{
	SetMainImageVisibility(false, false, false);
	SetDescription("");
}

void TitleScreen_Register(void)
{
	INFO_LOG("RegisterTitleScreen");
    GlobalState_Push(STATE_TITLE_SCREEN, 0, NULL);
    if(!GetTutorialSeen())
    {
        TriggerTutorialDialog(true);
        SetTutorialSeen(true);
    }
}
