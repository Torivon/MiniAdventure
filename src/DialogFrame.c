#include <pebble.h>
#include "Clock.h"
#include "DialogFrame.h"
#include "GlobalState.h"
#include "Logging.h"
#include "MiniAdventure.h"
#include "BaseWindow.h"
#include "TextBox.h"

static TextBox *dialogTextBox = NULL;
static TextBox *okTextBox = NULL;
static TextBox *cancelTextBox = NULL;

bool currentDialogAllowsCancel = false;

#define DIALOG_FRAME_WIDTH 100
#define DIALOG_FRAME_HEIGHT 80
#define OK_FRAME_WIDTH 25
#define OK_FRAME_HEIGHT 22
#define CANCEL_FRAME_WIDTH 25
#define CANCEL_FRAME_HEIGHT 22
#define DIALOG_TEXT_X_OFFSET 2
#define DIALOG_TEXT_Y_OFFSET 2
#if defined(PBL_RECT)
static GRect dialogFrame = {.origin = {.x = 0, .y = 168 / 2 - DIALOG_FRAME_HEIGHT / 2}, .size = {.w = DIALOG_FRAME_WIDTH, .h = DIALOG_FRAME_HEIGHT}};
static GRect okFrame = {.origin = {.x = 144 - 30, .y = 168 / 2 - OK_FRAME_HEIGHT / 2}, .size = {.w = OK_FRAME_WIDTH, .h = OK_FRAME_HEIGHT}};
static GRect cancelFrame = {.origin = {.x = 0, .y = 168 / 2 - CANCEL_FRAME_HEIGHT / 2}, .size = {.w = CANCEL_FRAME_WIDTH, .h = CANCEL_FRAME_HEIGHT}};
#elif defined(PBL_ROUND)
#define VERTICAL_OFFSET 20
static GRect dialogFrame = {.origin = {.x = 180 / 2 - DIALOG_FRAME_WIDTH / 2, .y = 180 / 2 - DIALOG_FRAME_HEIGHT / 2}, .size = {.w = DIALOG_FRAME_WIDTH, .h = DIALOG_FRAME_HEIGHT}};
static GRect okFrame = {.origin = {.x = 180 - 30, .y = 180 / 2 - OK_FRAME_HEIGHT / 2}, .size = {.w = OK_FRAME_WIDTH, .h = OK_FRAME_HEIGHT}};
static GRect cancelFrame = {.origin = {.x = 10, .y = 180 / 2 - CANCEL_FRAME_HEIGHT / 2}, .size = {.w = CANCEL_FRAME_WIDTH, .h = CANCEL_FRAME_HEIGHT}};
#endif

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
		dialogTextBox = CreateTextBox(DIALOG_TEXT_X_OFFSET, DIALOG_TEXT_Y_OFFSET, fonts_get_system_font(FONT_KEY_GOTHIC_14), dialogFrame);
		okTextBox = CreateTextBox(DIALOG_TEXT_X_OFFSET, DIALOG_TEXT_Y_OFFSET, fonts_get_system_font(FONT_KEY_GOTHIC_14), okFrame);
        cancelTextBox = CreateTextBox(DIALOG_TEXT_X_OFFSET, DIALOG_TEXT_Y_OFFSET, fonts_get_system_font(FONT_KEY_GOTHIC_14), cancelFrame);
	}
	
	InitializeTextBox(window, dialogTextBox, "");
	InitializeTextBox(window, okTextBox, "OK");
    InitializeTextBox(window, cancelTextBox, "X");
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
}

void DialogDisappear(void *data)
{
	HideDialogLayer();
}

void TriggerDialog(DialogData *data)
{
	GlobalState_Push(STATE_DIALOG, 0, NULL, NULL, DialogAppear, DialogDisappear, NULL, data);
}

void QueueDialog(DialogData *data)
{
    GlobalState_Queue(STATE_DIALOG, 0, NULL, NULL, DialogAppear, DialogDisappear, NULL, data);
}
