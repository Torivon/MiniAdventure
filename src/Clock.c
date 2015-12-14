#include <pebble.h>
#include "Clock.h"
#include "Logging.h"
#include "MiniAdventure.h"
#include "NewBaseWindow.h"
#include "TextBox.h"

static TextBox *clockTextBox = NULL;

#define CLOCK_FRAME_WIDTH 62
#define CLOCK_FRAME_HEIGHT 36
#define CLOCK_TEXT_X_OFFSET 6
#define CLOCK_TEXT_Y_OFFSET -1
#if defined(PBL_RECT)
static GRect newClockFrame = {.origin = {.x = 144 / 2 - CLOCK_FRAME_WIDTH / 2, .y = 168 - CLOCK_FRAME_HEIGHT}, .size = {.w = CLOCK_FRAME_WIDTH, .h = CLOCK_FRAME_HEIGHT}};
#elif defined(PBL_ROUND)
#define CLOCK_VERTICAL_OFFSET 10
static GRect newClockFrame = {.origin = {.x = 180 / 2 - CLOCK_FRAME_WIDTH / 2, .y = 180 - CLOCK_FRAME_HEIGHT - CLOCK_VERTICAL_OFFSET}, .size = {.w = CLOCK_FRAME_WIDTH, .h = CLOCK_FRAME_HEIGHT}};
#endif

char *UpdateClockText(void)
{
	static char timeText[] = "00:00"; // Needs to be static because it's used by the system later.
	char *time_format;

	if (clock_is_24h_style()) 
	{
		time_format = "%R";
	}
	else 
	{
		time_format = "%I:%M";
	}

	time_t now = time(NULL);
	struct tm *current_time = localtime(&now);
	
	strftime(timeText, sizeof(timeText), time_format, current_time);
	return timeText;
}

void UpdateNewClock(void)
{
	if(!TextBoxInitialized(clockTextBox) || !UsingNewWindow())
		return;
	
	TextBoxSetText(clockTextBox, UpdateClockText());
}

void RemoveNewClockLayer(void)
{
	RemoveTextBox(clockTextBox);
}

void InitializeNewClockLayer(Window *window)
{
	if(!clockTextBox)
	{
		clockTextBox = CreateTextBox(RESOURCE_ID_IMAGE_CLOCK_BACKGROUND, CLOCK_TEXT_X_OFFSET, CLOCK_TEXT_Y_OFFSET, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), newClockFrame);
	}
	
	InitializeTextBox(window, clockTextBox, UpdateClockText());
}

void FreeClockLayer(void)
{
	FreeTextBox(clockTextBox);
	clockTextBox = NULL;
}
