#include "pebble.h"

#include "ImageMap.h"
#include "AutoImageMap.h"

int ImageMap_GetIdByIndex(uint16_t index)
{
    return autoImageMap[index];
}