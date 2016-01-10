#include "pebble.h"

#include "Adventure.h"
#include "Credits.h"
#include "Character.h"
#include "DescriptionFrame.h"
#include "DialogFrame.h"
#include "ExtraMenu.h"
#include "GlobalState.h"
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

void SetTutorialSeen(bool enable)
{
    tutorialSeen = enable;
}

bool GetTutorialSeen(void)
{
    return tutorialSeen;
}

void LaunchResourceStory(uint16_t index)
{
    ResourceStory_SetCurrentStory(index);
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
            return LaunchResourceStory(index->row);
        case 1:
            return ExtraMenu_SelectAction(index->row);
    }
}

void TitleScreen_Appear(void *data)
{
    ReloadMenu(GetMainMenu());
	SetForegroundImage(RESOURCE_ID_IMAGE_TITLE);
	SetMainImageVisibility(true, true, false);
	SetDescription("MiniAdventure");
}

void TitleScreen_Pop(void *data)
{
	SetMainImageVisibility(false, false, false);
	SetDescription("");
}

DialogData introText[] =
{
    {
        .text = "MiniAdventure:\n Welcome to MiniAdventure. Press select to continue.",
        .allowCancel = true
    },
    {
        .text = "Use the select button to open the main menu.",
        .allowCancel = true
    },
    {
        .text = "Use the up and down buttons to make your selections inside menus.",
        .allowCancel = true
    },
    {
        .text = "The back button will exit games in progress.",
        .allowCancel = true
    },
};

void TitleScreen_Register(void)
{
	INFO_LOG("RegisterTitleScreen");
    GlobalState_Push(STATE_TITLE_SCREEN, 0, NULL);
    if(!GetTutorialSeen())
    {
        TriggerDialog(&introText[0]);
        QueueDialog(&introText[1]);
        QueueDialog(&introText[2]);
        QueueDialog(&introText[3]);
        SetTutorialSeen(true);
    }
}
