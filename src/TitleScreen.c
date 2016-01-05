#include "pebble.h"

#include "Adventure.h"
#include "Credits.h"
#include "Character.h"
#include "DescriptionFrame.h"
#include "DialogFrame.h"
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

void ChooseOptions(void)
{
    QueueOptionsScreen();
}

void ChooseRepo(void)
{
    QueueLargeImage(RESOURCE_ID_IMAGE_REPOSITORY_CODE, true);
}

static DialogData credits[] =
{
    {
        .text = "Programming and art by Jonathan Panttaja",
        .allowCancel = false
    },
    {
        .text = "Additional Contributors: Belphemur and BlackLamb",
        .allowCancel = false
    },
    {
        .text = "Code located at https://Github.com/Torivon/MiniAdventure",
        .allowCancel = false
    },
};

void ChooseCredits(void)
{
    QueueDialog(&credits[0]);
    QueueDialog(&credits[1]);
    QueueDialog(&credits[2]);
}

static uint16_t TitleScreenCount(void)
{
    uint16_t storyCount = GetStoryCount();
    return storyCount + 3;
}

static const char *TitleScreenNameCallback(int row)
{
    int storyCount = GetStoryCount();
    if(row < storyCount)
        return ResourceStory_GetNameByIndex(row);
    else
    {
        if(row == storyCount)
            return "Options";
        else if(row == storyCount + 1)
            return "Credits";
        else if(row == storyCount + 2)
            return "Repository";
    }
    return "None";
}

static const char *TitleScreenDescriptionCallback(int row)
{
    int storyCount = GetStoryCount();
    if(row < storyCount)
        return ResourceStory_GetDescriptionByIndex(row);
    else
    {
        if(row == storyCount)
            return "Options";
        else if(row == storyCount + 1)
            return "Credits";
        else if(row == storyCount + 2)
            return "Repository";
    }
    return "None";
}

static void TitleScreenSelectCallback(int row)
{
    int storyCount = GetStoryCount();
    if(row < storyCount)
        LaunchResourceStory(row);
    else
    {
        if(row == storyCount)
            ChooseOptions();
        else if(row == storyCount + 1)
            ChooseCredits();
        else if(row == storyCount + 2)
            ChooseRepo();
    }
}

static void TitleScreenAppear(void *data)
{
    RegisterMenuCellCallbacks(GetMainMenu(), TitleScreenCount, TitleScreenNameCallback, TitleScreenDescriptionCallback, TitleScreenSelectCallback);
	SetForegroundImage(RESOURCE_ID_IMAGE_TITLE);
	SetMainImageVisibility(true, true, false);
	SetDescription("MiniAdventure");
}

static void TitleScreenPop(void *data)
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

void RegisterTitleScreen(void)
{
	INFO_LOG("RegisterTitleScreen");
    GlobalState_Push(STATE_TITLE_SCREEN, 0, NULL, NULL, TitleScreenAppear, NULL, TitleScreenPop, NULL);
    if(!GetTutorialSeen())
    {
        TriggerDialog(&introText[0]);
        QueueDialog(&introText[1]);
        QueueDialog(&introText[2]);
        QueueDialog(&introText[3]);
        SetTutorialSeen(true);
    }
}
