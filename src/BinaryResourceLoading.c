#include <pebble.h>
#include "MiniAdventure.h"
#include "BinaryResourceLoading.h"
#include "Logging.h"


uint16_t ResourceLoad16BitInt(ResHandle data, int *index)
{
    uint8_t int_bytes[2] = {0};
    resource_load_byte_range(data, *index, int_bytes, 2);
    *index += 2;
    return (int_bytes[1] << 8) + int_bytes[0];
}

void ResourceLoadString(ResHandle data, int index, char *buffer, int length)
{
    resource_load_byte_range(data, index, (uint8_t*)buffer, length);
}

void ResourceLoadStruct(ResHandle data, int logical_index, uint8_t *buffer, uint16_t expected_size, const char *structName)
{
    int start_index = ResourceLoad_GetByteIndexFromLogicalIndex(logical_index);
    int read_index = ResourceLoad16BitInt(data, &start_index);
    int size = ResourceLoad16BitInt(data, &start_index);
    
    if(expected_size != size && structName)
    {
        ERROR_LOG("Size mismatch loading %s: got %d, expected %d", structName, size, expected_size);
    }
    
    resource_load_byte_range(data, read_index, buffer, size);
}

uint16_t ResourceLoad_GetByteIndexFromLogicalIndex(uint16_t index)
{
    return (1 + index * 2) * 2;
}
