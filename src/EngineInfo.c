#include "pebble.h"
#include "BinaryResourceLoading.h"
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

