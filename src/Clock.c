#include <pebble.h>
#include "Clock.h"
#include "Logging.h"
#include "MiniAdventure.h"
#include "BaseWindow.h"
#include "TextBox.h"

static TextBox *clockTextBox = NULL;
static TextBox *dateTextBox = NULL;
static TextBox *dayTextBox = NULL;

#define CLOCK_FRAME_WIDTH 62
#define CLOCK_FRAME_HEIGHT 36
#define CLOCK_TEXT_X_OFFSET 6
#define CLOCK_TEXT_Y_OFFSET -1
#if defined(PBL_RECT)
static GRect clockFrame = {.origin = {.x = 144 / 2 - CLOCK_FRAME_WIDTH / 2, .y = 168 - CLOCK_FRAME_HEIGHT}, .size = {.w = CLOCK_FRAME_WIDTH, .h = CLOCK_FRAME_HEIGHT}};
static GRect dayFrame = {.origin = {.x = 0, .y = 60}, .size = {.w = 30, .h = 18}};
static GRect dateFrame = {.origin = {.x = 0, .y = 100}, .size = {.w = 60, .h = 18}};
#elif defined(PBL_ROUND)
#define CLOCK_VERTICAL_OFFSET 10
static GRect clockFrame = {.origin = {.x = 180 / 2 - CLOCK_FRAME_WIDTH / 2, .y = 180 - CLOCK_FRAME_HEIGHT - CLOCK_VERTICAL_OFFSET}, .size = {.w = CLOCK_FRAME_WIDTH, .h = CLOCK_FRAME_HEIGHT}};
static GRect dayFrame = {.origin = {.x = 45, .y = 48}, .size = {.w = 30, .h = 18}};
static GRect dateFrame = {.origin = {.x = 15, .y = 114}, .size = {.w = 60, .h = 18}};
#endif

char *UpdateDateText(void)
{
    static char dateText[] = "00/00/00";
    char *date_format;
    
    date_format = "%m/%d/%y";
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    
    strftime(dateText, sizeof(dateText), date_format, current_time);
    return dateText;
}

char *UpdateDayText(void)
{
    static char dayText[] = "Sun";
    char *day_format;
    
    day_format = "%a";
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    
    strftime(dayText, sizeof(dayText), day_format, current_time);
    return dayText;
}

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

void UpdateClock(void)
{
    if(!TextBoxInitialized(clockTextBox))
        return;
    
    TextBoxSetText(clockTextBox, UpdateClockText());
    TextBoxSetText(dayTextBox, UpdateDayText());
    TextBoxSetText(dateTextBox, UpdateDateText());
}

void RemoveClockLayer(void)
{
    RemoveTextBox(clockTextBox);
    RemoveTextBox(dayTextBox);
    RemoveTextBox(dateTextBox);
}

void HideDateLayer(void)
{
    HideTextBox(dayTextBox);
    HideTextBox(dateTextBox);
}

void ShowDateLayer(void)
{
    ShowTextBox(dayTextBox);
    ShowTextBox(dateTextBox);
}

void InitializeClockLayer(Window *window)
{
    if(!clockTextBox)
    {
        clockTextBox = CreateTextBox(CLOCK_TEXT_X_OFFSET, CLOCK_TEXT_Y_OFFSET, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), clockFrame);
        dayTextBox = CreateTextBox(0, 0, fonts_get_system_font(FONT_KEY_GOTHIC_14), dayFrame);
        dateTextBox = CreateTextBox(0, 0, fonts_get_system_font(FONT_KEY_GOTHIC_14), dateFrame);
    }
    
    InitializeTextBox(window, clockTextBox, UpdateClockText());
    InitializeTextBox(window, dayTextBox, UpdateDayText());
    InitializeTextBox(window, dateTextBox, UpdateDateText());
}

void FreeClockLayer(void)
{
    FreeTextBox(clockTextBox);
    FreeTextBox(dayTextBox);
    FreeTextBox(dateTextBox);
    clockTextBox = NULL;
    dayTextBox = NULL;
    dateTextBox = NULL;
}
