#pragma once

void SetDescription(const char *text);
const char *GetDescription(void);
void RemoveDescriptionLayer(void);
void InitializeDescriptionLayer(Window *window);
void FreeDescriptionLayer(void);
