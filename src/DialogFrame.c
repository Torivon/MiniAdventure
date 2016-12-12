#include <pebble.h>
#include "BinaryResourceLoading.h"
#include "Clock.h"
#include "DescriptionFrame.h"
#include "DialogFrame.h"
#include "GlobalState.h"
#include "Logging.h"
#include "MiniAdventure.h"
#include "BaseWindow.h"
#include "TextBox.h"
#include "Utils.h"

typedef struct DialogData
{
    char name[MAX_STORY_NAME_LENGTH];
    char text[MAX_DIALOG_LENGTH];
    uint16_t allowCancel;
    uint16_t heap;
} DialogData;

static TextBox *dialogTextBox = NULL;
static TextBox *okTextBox = NULL;
static TextBox *cancelTextBox = NULL;

bool currentDialogAllowsCancel = false;

#define DIALOG_FRAME_WIDTH 100
#define DIALOG_FRAME_HEIGHT 80
#define CANCEL_FRAME_WIDTH 20
#define CANCEL_FRAME_HEIGHT 22
#define DIALOG_TEXT_X_OFFSET 3
#define DIALOG_TEXT_Y_OFFSET 2
#if defined(PBL_RECT)
static GRect dialogFrame = {.origin = {.x = 144 / 2 - DIALOG_FRAME_WIDTH / 2, .y = 168 / 2 - DIALOG_FRAME_HEIGHT / 2}, .size = {.w = DIALOG_FRAME_WIDTH, .h = DIALOG_FRAME_HEIGHT}};
static GRect okFrame = {.origin = {.x = 144 - OK_FRAME_WIDTH, .y = 168 / 2 - OK_FRAME_HEIGHT / 2}, .size = {.w = OK_FRAME_WIDTH, .h = OK_FRAME_HEIGHT}};
static GRect cancelFrame = {.origin = {.x = 0, .y = 168 / 4 - CANCEL_FRAME_HEIGHT / 2}, .size = {.w = CANCEL_FRAME_WIDTH, .h = CANCEL_FRAME_HEIGHT}};
#elif defined(PBL_ROUND)
static GRect dialogFrame = {.origin = {.x = 180 / 2 - DIALOG_FRAME_WIDTH / 2, .y = 180 / 2 - DIALOG_FRAME_HEIGHT / 2}, .size = {.w = DIALOG_FRAME_WIDTH, .h = DIALOG_FRAME_HEIGHT}};
static GRect okFrame = {.origin = {.x = 180 - OK_FRAME_WIDTH - 5, .y = 180 / 2 - OK_FRAME_HEIGHT / 2}, .size = {.w = OK_FRAME_WIDTH, .h = OK_FRAME_HEIGHT}};
static GRect cancelFrame = {.origin = {.x = 10, .y = 180 / 2 - CANCEL_FRAME_HEIGHT / 2}, .size = {.w = CANCEL_FRAME_WIDTH, .h = CANCEL_FRAME_HEIGHT}};
#endif

DialogData *DialogData_Load(ResHandle handle, uint16_t dialogIndex)
{
    if(dialogIndex == 0)
        return NULL;
    
    DialogData *dialog = calloc(sizeof(DialogData), 1);
    ResourceLoadStruct(handle, dialogIndex, (uint8_t*)dialog, sizeof(DialogData), "DialogData");
    return dialog;
}

DialogData *DialogData_Create(const char *name, const char *text, uint16_t allowCancel)
{
    DialogData *dialog = calloc(sizeof(DialogData), 1);
    dialog->heap = true;
    dialog->allowCancel = allowCancel;
    snprintf(dialog->name, MAX_STORY_NAME_LENGTH, "%s", name);
    snprintf(dialog->text, MAX_DIALOG_LENGTH, "%s", text);
    return dialog;
}

void SetDialog(const char *text)
{
	TextBoxSetText(dialogTextBox, text);
}

const char *GetDialog(void)
{
	return TextBoxGetText(dialogTextBox);
}

bool Dialog_AllowCancel(void)
{
    return currentDialogAllowsCancel;
}

void RemoveDialogLayer(void)
{
	RemoveTextBox(dialogTextBox);
	RemoveTextBox(okTextBox);
    RemoveTextBox(cancelTextBox);
}

void InitializeDialogLayer(Window *window)
{
	if(!dialogTextBox)
	{
		dialogTextBox = CreateTextBox(DIALOG_TEXT_X_OFFSET, DIALOG_TEXT_Y_OFFSET, fonts_get_system_font(FONT_KEY_GOTHIC_14), dialogFrame, GTextAlignmentLeft, true, false);
		okTextBox = CreateTextBox(DIALOG_TEXT_X_OFFSET, DIALOG_TEXT_Y_OFFSET, fonts_get_system_font(FONT_KEY_GOTHIC_14), okFrame, GTextAlignmentCenter, false, false);
        cancelTextBox = CreateTextBox(DIALOG_TEXT_X_OFFSET, DIALOG_TEXT_Y_OFFSET, fonts_get_system_font(FONT_KEY_GOTHIC_14), cancelFrame, GTextAlignmentCenter, false, false);
	}
	
	InitializeTextBox(window_get_root_layer(window), dialogTextBox, "");
	InitializeTextBox(window_get_root_layer(window), okTextBox, "OK");
    InitializeTextBox(window_get_root_layer(window), cancelTextBox, "X");
	HideDialogLayer();
	
}

void FreeDialogLayer(void)
{
	FreeTextBox(dialogTextBox);
	dialogTextBox = NULL;
	FreeTextBox(okTextBox);
	okTextBox = NULL;
    FreeTextBox(cancelTextBox);
    cancelTextBox = NULL;
}

void ShowDialogLayer(bool allowCancel)
{
	ShowTextBox(dialogTextBox);
	ShowTextBox(okTextBox);
    if(allowCancel)
        ShowTextBox(cancelTextBox);
    currentDialogAllowsCancel = allowCancel;
}

void HideDialogLayer(void)
{
	HideTextBox(dialogTextBox);
	HideTextBox(okTextBox);	
    HideTextBox(cancelTextBox);
}

void DialogAppear(void *data)
{
    DialogData *dialogData = (DialogData*) data;
    SetDialog(dialogData->text);
	ShowDialogLayer(dialogData->allowCancel);
    SetDescription(dialogData->name);
}

void DialogDisappear(void *data)
{
	HideDialogLayer();
}

void Dialog_Pop(void *data)
{
    DialogData *frame = (DialogData*)data;
    if(frame->heap)
    {
        free(frame);
    }
}

void DialogFrame_ScrollUp(void)
{
    TextBox_ScrollUp(dialogTextBox);
}

void DialogFrame_ScrollDown(void)
{
    TextBox_ScrollDown(dialogTextBox);
}

void Dialog_Trigger(DialogData *data)
{
	GlobalState_Push(STATE_DIALOG, 0, data);
}

void Dialog_TriggerFromResource(ResHandle handle, uint16_t dialogIndex)
{
    DialogData *dialog = DialogData_Load(handle, dialogIndex);
    if(!dialog)
        return;
    Dialog_Trigger(dialog);
}

void Dialog_Queue(DialogData *data)
{
    GlobalState_Queue(STATE_DIALOG, 0, data);
}

void Dialog_QueueFromResource(ResHandle handle, uint16_t dialogIndex)
{
    DialogData *dialog = DialogData_Load(handle, dialogIndex);
    if(!dialog)
        return;
    Dialog_Queue(dialog);
}
