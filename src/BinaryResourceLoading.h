#pragma once

uint16_t ResourceLoad16BitInt(ResHandle data, int *index);
void ResourceLoadString(ResHandle data, int *index, char *buffer, int length);
void ResourceLoadStruct(ResHandle data, int logical_index, uint8_t *buffer, uint16_t expected_size, const char *structName);

uint16_t ResourceLoad_GetByteIndexFromLogicalIndex(uint16_t index);
