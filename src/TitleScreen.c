#include "pebble.h"

#include "Adventure.h"
#include "BinaryResourceLoading.h"
#include "Character.h"
#include "DescriptionFrame.h"
#include "DialogFrame.h"
#include "EngineInfo.h"
#include "ExtraMenu.h"
#include "GlobalState.h"
#include "ImageMap.h"
#include "LargeImage.h"
#include "Logging.h"
#include "MainImage.h"
#include "Menu.h"
#include "MiniAdventure.h"
#include "OptionsMenu.h"
#include "BaseWindow.h"
#include "StoryList.h"
#include "ResourceStory.h"

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

void LaunchResourceStory(uint16_t index, bool now)
{
    ResourceStory_SetCurrentStory(index);
    if(now)
        TriggerAdventureScreen();
    else
        QueueAdventureScreen();
}

uint16_t TitleScreen_MenuSectionCount(void)
{
    return 1 + ExtraMenu_GetSectionCount();
}

const char *TitleScreen_MenuSectionName(uint16_t sectionIndex)
{
    switch(sectionIndex)
    {
        case 0:
            return "Stories";
        case 1:
            return ExtraMenu_GetSectionName();
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
            return ExtraMenu_GetCellCount();
    }
    return 0;
}

const char *TitleScreen_MenuCellName(MenuIndex *index)
{
    switch(index->section)
    {
        case 0:
            return ResourceStory_GetNameByIndex(index->row);
        case 1:
            return ExtraMenu_GetCellName(index->row);
    }
    return "None";
}

const char *TitleScreen_MenuCellDescription(MenuIndex *index)
{
    switch(index->section)
    {
        case 0:
            return ResourceStory_GetDescriptionByIndex(index->row);
        case 1:
            return ExtraMenu_GetCellName(index->row);
    }
    return "None";
}

void TitleScreen_MenuSelect(MenuIndex *index)
{
    switch(index->section)
    {
        case 0:
            return LaunchResourceStory(index->row, false);
        case 1:
            return ExtraMenu_SelectAction(index->row);
    }
}

void TitleScreen_Appear(void *data)
{
    RegisterMenuState(GetMainMenu(), STATE_TITLE_SCREEN);
    RegisterMenuState(GetSlaveMenu(), STATE_NONE);
	SetForegroundImage(ImageMap_GetIdByIndex(EngineInfo_GetInfo()->titleImage));
	SetMainImageVisibility(true, true, false);
	SetDescription("MiniAdventure");
    if(launch_reason() == APP_LAUNCH_WORKER && ResourceStory_IsLastResourceStoryIdValid() && firstLaunch)
    {
        LaunchResourceStory(ResourceStory_GetStoryIndexById(ResourceStory_GetLastResourceStoryId()), true);
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
