#include "pebble.h"

#include "DialogFrame.h"
#include "ExtraMenu.h"
#include "LargeImage.h"
#include "OptionsMenu.h"



uint16_t ExtraMenu_GetSectionCount(void)
{
    return 1;
}

const char *ExtraMenu_GetSectionName(void)
{
    return "Extras";
}

uint16_t ExtraMenu_GetCellCount(void)
{
    return 3;
}

const char *ExtraMenu_GetCellName(uint16_t row)
{
    switch(row)
    {
        case 0:
            return "Options";
        case 1:
            return "Credits";
        case 2:
            return "Repository";
        default:
            return "None";
    }
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
            QueueDialog(&credits[0]);
            QueueDialog(&credits[1]);
            QueueDialog(&credits[2]);
            break;
        }
        case 2:
        {
            QueueLargeImage(RESOURCE_ID_IMAGE_REPOSITORY_CODE, true);
            break;
        }
    }
}