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

#define DESC_FRAME_WIDTH 84
#define DESC_FRAME_HEIGHT 30
#define DESC_TEXT_X_OFFSET 2
#define DESC_TEXT_Y_OFFSET 2
#if defined(PBL_RECT)
static GRect dialogFrame = {.origin = {.x = 144 / 2 - DESC_FRAME_WIDTH / 2, .y = 0}, .size = {.w = DESC_FRAME_WIDTH, .h = DESC_FRAME_HEIGHT}};
static GRect okFrame = {.origin = {.x = 144 - 40, .y = 70}, .size = {.w = DESC_FRAME_WIDTH, .h = DESC_FRAME_HEIGHT}};
#elif defined(PBL_ROUND)
#define VERTICAL_OFFSET 10
static GRect dialogFrame = {.origin = {.x = 180 / 2 - DESC_FRAME_WIDTH / 2, .y = VERTICAL_OFFSET}, .size = {.w = DESC_FRAME_WIDTH, .h = DESC_FRAME_HEIGHT}};
static GRect okFrame = {.origin = {.x = 180 - 40, .y = 80}, .size = {.w = DESC_FRAME_WIDTH, .h = DESC_FRAME_HEIGHT}};
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
		dialogTextBox = CreateTextBox(RESOURCE_ID_IMAGE_DIALOG_FRAME, DESC_TEXT_X_OFFSET, DESC_TEXT_Y_OFFSET, fonts_get_system_font(FONT_KEY_GOTHIC_14), dialogFrame);
		okTextBox = CreateTextBox(RESOURCE_ID_IMAGE_OK_FRAME, DESC_TEXT_X_OFFSET, DESC_TEXT_Y_OFFSET, fonts_get_system_font(FONT_KEY_GOTHIC_14), okFrame);
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