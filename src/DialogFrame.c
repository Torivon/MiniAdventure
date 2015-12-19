#include <pebble.h>
#include "Clock.h"
#include "DialogFrame.h"
#include "GlobalState.h"
#include "Logging.h"
#include "MiniAdventure.h"
#include "NewBaseWindow.h"
#include "TextBox.h"

static TextBox *dialogTextBox = NULL;
static TextBox *okTextBox = NULL;

#define DIALOG_FRAME_WIDTH 100
#define DIALOG_FRAME_HEIGHT 80
#define OK_FRAME_WIDTH 25
#define OK_FRAME_HEIGHT 22
#define DIALOG_TEXT_X_OFFSET 2
#define DIALOG_TEXT_Y_OFFSET 2
#if defined(PBL_RECT)
static GRect dialogFrame = {.origin = {.x = 0, .y = 168 / 2 - DIALOG_FRAME_HEIGHT / 2}, .size = {.w = DIALOG_FRAME_WIDTH, .h = DIALOG_FRAME_HEIGHT}};
static GRect okFrame = {.origin = {.x = 144 - 30, .y = 168 / 2 - OK_FRAME_HEIGHT / 2}, .size = {.w = OK_FRAME_WIDTH, .h = OK_FRAME_HEIGHT}};
#elif defined(PBL_ROUND)
#define VERTICAL_OFFSET 20
static GRect dialogFrame = {.origin = {.x = VERTICAL_OFFSET, .y = 180 / 2 - DIALOG_FRAME_HEIGHT / 2}, .size = {.w = DIALOG_FRAME_WIDTH, .h = DIALOG_FRAME_HEIGHT}};
static GRect okFrame = {.origin = {.x = 180 - 30, .y = 180 / 2 - OK_FRAME_HEIGHT / 2}, .size = {.w = OK_FRAME_WIDTH, .h = OK_FRAME_HEIGHT}};
#endif

void SetDialog(const char *text)
{
	if(!TextBoxInitialized(dialogTextBox))
		return;
	
	DEBUG_LOG("SetDialog %s", text);
	TextBoxSetText(dialogTextBox, text);
}

const char *GetDialog(void)
{
	if(!TextBoxInitialized(dialogTextBox))
		return "";
	
	return TextBoxGetText(dialogTextBox);
}

void RemoveDialogLayer(void)
{
	RemoveTextBox(dialogTextBox);
	RemoveTextBox(okTextBox);
}

void InitializeDialogLayer(Window *window)
{
	if(!dialogTextBox)
	{
		dialogTextBox = CreateTextBox(DIALOG_TEXT_X_OFFSET, DIALOG_TEXT_Y_OFFSET, fonts_get_system_font(FONT_KEY_GOTHIC_14), dialogFrame);
		okTextBox = CreateTextBox(DIALOG_TEXT_X_OFFSET, DIALOG_TEXT_Y_OFFSET, fonts_get_system_font(FONT_KEY_GOTHIC_14), okFrame);
	}
	
	InitializeTextBox(window, dialogTextBox, "");
	InitializeTextBox(window, okTextBox, "OK");
	HideDialogLayer();
	
}

void FreeDialogLayer(void)
{
	FreeTextBox(dialogTextBox);
	dialogTextBox = NULL;
	FreeTextBox(okTextBox);
	okTextBox = NULL;
}

void ShowDialogLayer(void)
{
	ShowTextBox(dialogTextBox);
	ShowTextBox(okTextBox);
}

void HideDialogLayer(void)
{
	HideTextBox(dialogTextBox);
	HideTextBox(okTextBox);	
}

void DialogAppear(void *data)
{
	ShowDialogLayer();
}

void DialogDisappear(void *data)
{
	HideDialogLayer();
}

void TriggerDialog(const char *text)
{
	SetDialog(text);
	PushGlobalState(STATE_DIALOG, 0, NULL, NULL, DialogAppear, DialogDisappear, NULL, NULL);
}