#pragma once

uint16_t ResourceLoad16BitInt(ResHandle data, int *index);
void ResourceLoadString(ResHandle data, int *index, char *buffer, int length);

uint16_t ResourceLoad_GetByteIndexFromLogicalIndex(uint16_t index);
