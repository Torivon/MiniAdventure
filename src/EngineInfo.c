#include <pebble.h>
#include "BinaryResourceLoading.h"
#include "DialogFrame.h"
#include "ImageMap.h"
#include "EngineInfo.h"

EngineInfo engineInfo = {0};
static bool infoLoaded = false;

ResHandle EngineInfo_GetResHandle(void)
{
    return resource_get_handle(RESOURCE_ID_ENGINEINFO);
}

void EngineInfo_Load(void)
{
    ResourceLoadStruct(EngineInfo_GetResHandle(), 0, (uint8_t*)&engineInfo, sizeof(EngineInfo), "EngineInfo");
    infoLoaded = true;
}

EngineInfo *EngineInfo_GetInfo(void)
{
    if(!infoLoaded)
        EngineInfo_Load();
    
    return &engineInfo;
}

void EngineInfo_QueueWinDialog(void)
{
    Dialog_QueueFromResource(EngineInfo_GetResHandle(), EngineInfo_GetInfo()->battleWinDialog);
}

void EngineInfo_QueueLevelUpDialog(void)
{
    Dialog_QueueFromResource(EngineInfo_GetResHandle(), EngineInfo_GetInfo()->levelUpDialog);
}

void EngineInfo_QueueResetDialog(void)
{
    Dialog_QueueFromResource(EngineInfo_GetResHandle(), EngineInfo_GetInfo()->resetPromptDialog);
}
