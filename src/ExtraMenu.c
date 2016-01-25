#include "pebble.h"

#include "BaseWindow.h"
#include "BinaryResourceLoading.h"
#include "DialogFrame.h"
#include "EngineInfo.h"
#include "ExtraMenu.h"
#include "GlobalState.h"
#include "Menu.h"
#include "ImageMap.h"
#include "LargeImage.h"
#include "OptionsMenu.h"

void TriggerTutorialDialog(bool now)
{
    DialogData *dialog = calloc(sizeof(DialogData), 1);
    ResourceLoadStruct(EngineInfo_GetResHandle(), EngineInfo_GetInfo()->tutorialDialog, (uint8_t*)dialog, sizeof(DialogData), "DialogData");
    if(now)
        TriggerDialog(dialog);
    else
        QueueDialog(dialog);
}

uint16_t ExtraMenu_GetSectionCount(void)
{
    return 1;
}

const char *ExtraMenu_GetSectionName(void)
{
    return "Engine";
}

uint16_t ExtraMenu_GetCellCount(void)
{
    return 4;
}

const char *ExtraMenu_GetCellName(uint16_t row)
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

void ExtraMenu_SelectAction(uint16_t row)
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
            DialogData *dialog = calloc(sizeof(DialogData), 1);
            ResourceLoadStruct(EngineInfo_GetResHandle(), EngineInfo_GetInfo()->engineCreditsDialog, (uint8_t*)dialog, sizeof(DialogData), "DialogData");
            QueueDialog(dialog);
            break;
        }
        case 3:
        {
            QueueLargeImage(ImageMap_GetIdByIndex(EngineInfo_GetInfo()->engineRepositoryImage), true);
            break;
        }
    }
}

void ExtraMenu_SubMenu_Trigger(void)
{
    QueueRegisterMenuState(GetMainMenu(), STATE_EXTRA_MENU);
    QueueMenu(GetMainMenu());
}
