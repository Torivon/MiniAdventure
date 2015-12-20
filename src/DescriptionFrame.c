#include <pebble.h>
#include "Clock.h"
#include "Logging.h"
#include "MiniAdventure.h"
#include "NewBaseWindow.h"
#include "TextBox.h"

static TextBox *descriptionTextBox = NULL;

#define DESC_FRAME_WIDTH 84
#define DESC_FRAME_HEIGHT 35
#define DESC_TEXT_X_OFFSET 2
#define DESC_TEXT_Y_OFFSET 2
#if defined(PBL_RECT)
static GRect descriptionFrame = {.origin = {.x = 144 / 2 - DESC_FRAME_WIDTH / 2, .y = 0}, .size = {.w = DESC_FRAME_WIDTH, .h = DESC_FRAME_HEIGHT}};
#elif defined(PBL_ROUND)
#define VERTICAL_OFFSET 11
static GRect descriptionFrame = {.origin = {.x = 180 / 2 - DESC_FRAME_WIDTH / 2, .y = VERTICAL_OFFSET}, .size = {.w = DESC_FRAME_WIDTH, .h = DESC_FRAME_HEIGHT}};
#endif

void SetDescription(const char *text)
{
	if(!TextBoxInitialized(descriptionTextBox))
		return;
	
	DEBUG_LOG("SetDescription %s", text);
	TextBoxSetText(descriptionTextBox, text);
}

const char *GetDescription(void)
{
	if(!TextBoxInitialized(descriptionTextBox))
		return "";
	
	return TextBoxGetText(descriptionTextBox);
}

void RemoveDescriptionLayer(void)
{
	RemoveTextBox(descriptionTextBox);
}

void InitializeDescriptionLayer(Window *window)
{
	if(!descriptionTextBox)
	{
		descriptionTextBox = CreateTextBox(DESC_TEXT_X_OFFSET, DESC_TEXT_Y_OFFSET, fonts_get_system_font(FONT_KEY_GOTHIC_14), descriptionFrame);
	}
	
	InitializeTextBox(window, descriptionTextBox, "");
}

void FreeDescriptionLayer(void)
{
	FreeTextBox(descriptionTextBox);
	descriptionTextBox = NULL;
}
