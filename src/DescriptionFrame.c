#include <pebble.h>
#include "Clock.h"
#include "Logging.h"
#include "MiniAdventure.h"
#include "BaseWindow.h"
#include "TextBox.h"

static TextBox *descriptionTextBox = NULL;

#if defined(PBL_RECT)
static GRect descriptionFrame = {.origin = {.x = DESC_FRAME_ORIGIN_X, .y = 0}, .size = {.w = DESC_FRAME_WIDTH, .h = DESC_FRAME_HEIGHT}};
#elif defined(PBL_ROUND)
static GRect descriptionFrame = {.origin = {.x = DESC_FRAME_ORIGIN_X, .y = VERTICAL_OFFSET}, .size = {.w = DESC_FRAME_WIDTH, .h = DESC_FRAME_HEIGHT}};
#endif

void SetDescription(const char *text)
{
	TextBoxSetText(descriptionTextBox, text);
}

const char *GetDescription(void)
{
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
		descriptionTextBox = CreateTextBox(DESC_TEXT_X_OFFSET, DESC_TEXT_Y_OFFSET, fonts_get_system_font(FONT_KEY_GOTHIC_14), descriptionFrame, GTextAlignmentCenter, false, PBL_IF_RECT_ELSE(false, true));
	}
	
	InitializeTextBox(window_get_root_layer(window), descriptionTextBox, "");
}

void FreeDescriptionLayer(void)
{
	FreeTextBox(descriptionTextBox);
	descriptionTextBox = NULL;
}
