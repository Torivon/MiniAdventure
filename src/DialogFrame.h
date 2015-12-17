#pragma once

void SetDialog(const char *text);
const char *GetDialog(void);
void RemoveDialogLayer(void);
void InitializeDialogLayer(Window *window);
void FreeDialogLayer(void);
void ShowDialogLayer(void);
void HideDialogLayer(void);

void TriggerDialog(const char *text);
