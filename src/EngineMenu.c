#include <pebble.h>

#include "BaseWindow.h"
#include "BinaryResourceLoading.h"
#include "DialogFrame.h"
#include "EngineInfo.h"
#include "EngineMenu.h"
#include "GlobalState.h"
#include "Menu.h"
#include "ImageMap.h"
#include "LargeImage.h"
#include "OptionsMenu.h"

void TriggerTutorialDialog(bool now)
{
    if(now)
        Dialog_TriggerFromResource(EngineInfo_GetResHandle(), EngineInfo_GetInfo()->tutorialDialog);
    else
        Dialog_QueueFromResource(EngineInfo_GetResHandle(), EngineInfo_GetInfo()->tutorialDialog);
}

uint16_t EngineMenu_GetSectionCount(void)
{
    return 1;
}

const char *EngineMenu_GetSectionName(void)
{
    return "Engine";
}

uint16_t EngineMenu_GetCellCount(void)
{
    return 4;
}

const char *EngineMenu_GetCellName(uint16_t row)
{
    switch(row)
    {
        case 0:
            return "Options";
        case 1:
            return "Tutorial";
        case 2:
            return "Credits";
        case 3:
            return "Repository";
        default:
            return "None";
    }
}

void EngineMenu_SelectAction(uint16_t row)
{
    switch(row)
    {
        case 0:
        {
            QueueOptionsScreen();
            break;
        }
        case 1:
        {
            TriggerTutorialDialog(false);
            break;
        }
        case 2:
        {
            Dialog_QueueFromResource(EngineInfo_GetResHandle(), EngineInfo_GetInfo()->engineCreditsDialog);
            break;
        }
        case 3:
        {
            QueueLargeImage(ImageMap_GetIdByIndex(EngineInfo_GetInfo()->engineRepositoryImage), true);
            break;
        }
    }
}

void EngineMenu_SubMenu_Trigger(void)
{
    QueueRegisterMenuState(GetMainMenu(), STATE_ENGINE_MENU);
    QueueMenu(GetMainMenu());
}
