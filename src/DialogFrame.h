#pragma once

typedef struct DialogData
{
    char *text;
    bool allowCancel;
    bool heap;
} DialogData;

void SetDialog(const char *text);
const char *GetDialog(void);
void RemoveDialogLayer(void);
void InitializeDialogLayer(Window *window);
void FreeDialogLayer(void);
void ShowDialogLayer(bool allowCancel);
void HideDialogLayer(void);

bool Dialog_AllowCancel(void);
void Dialog_Pop(void *data);

void TriggerDialog(DialogData *data);
void QueueDialog(DialogData *data);

void DialogAppear(void *data);
void DialogDisappear(void *data);

void DialogFrame_ScrollUp(void);
void DialogFrame_ScrollDown(void);
