#pragma once

typedef struct DialogData
{
    const char *text;
    const bool allowCancel;
} DialogData;

void SetDialog(const char *text);
const char *GetDialog(void);
void RemoveDialogLayer(void);
void InitializeDialogLayer(Window *window);
void FreeDialogLayer(void);
void ShowDialogLayer(bool allowCancel);
void HideDialogLayer(void);

bool Dialog_AllowCancel(void);

void TriggerDialog(DialogData *data);
void QueueDialog(DialogData *data);