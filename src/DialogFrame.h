#pragma once
#include "AutoSizeConstants.h"

typedef struct DialogData DialogData;

DialogData *DialogData_Load(ResHandle handle, uint16_t dialogIndex);
DialogData *DialogData_Create(const char *name, const char *text, uint16_t allowCancel);

void SetDialog(const char *text);
const char *GetDialog(void);
void RemoveDialogLayer(void);
void InitializeDialogLayer(Window *window);
void FreeDialogLayer(void);
void ShowDialogLayer(bool allowCancel);
void HideDialogLayer(void);

bool Dialog_AllowCancel(void);
void Dialog_Pop(void *data);

void Dialog_Trigger(DialogData *data);
void Dialog_TriggerFromResource(ResHandle handle, uint16_t dialogIndex);
void Dialog_Queue(DialogData *data);
void Dialog_QueueFromResource(ResHandle handle, uint16_t dialogIndex);

void DialogAppear(void *data);
void DialogDisappear(void *data);

void DialogFrame_ScrollUp(void);
void DialogFrame_ScrollDown(void);
