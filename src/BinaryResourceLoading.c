#include "pebble.h"
#include "MiniAdventure.h"
#include "BinaryResourceLoading.h"


uint16_t ResourceLoad16BitInt(ResHandle data, int index)
{
    uint8_t int_bytes[2] = {0};
    resource_load_byte_range(data, index, int_bytes, 2);
    return (int_bytes[1] << 8) + int_bytes[0];
}

void ResourceLoadString(ResHandle data, int index, char *buffer, int length)
{
    resource_load_byte_range(data, index, (uint8_t*)buffer, length);
}

