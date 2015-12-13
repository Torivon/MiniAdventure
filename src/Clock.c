#include <pebble.h>
#include "Clock.h"
#include "Logging.h"
#include "MiniAdventure.h"
#include "NewBaseWindow.h"

//******* CLOCK *********//

static BitmapLayer *newClockBackgroundLayer;
static GBitmap *newClockBackgroundImage;
static TextLayer *newClockLayer; // The clock
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
static bool newClockLayerInitialized = false;

void UpdateNewClock(void)
{
	if(!newClockLayerInitialized || !UsingNewWindow())
		return;

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

	text_layer_set_text(newClockLayer, timeText);
}

void RemoveNewClockLayer(void)
{
	if(!newClockLayerInitialized)
		return;

	layer_remove_from_parent(bitmap_layer_get_layer(newClockBackgroundLayer));
}

void InitializeNewClockLayer(Window *window)
{
	Layer *window_layer = window_get_root_layer(window);
	if(!newClockLayerInitialized)
	{
		newClockBackgroundImage = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLOCK_BACKGROUND);
		newClockBackgroundLayer = bitmap_layer_create(newClockFrame);
		bitmap_layer_set_bitmap(newClockBackgroundLayer, newClockBackgroundImage);
		bitmap_layer_set_alignment(newClockBackgroundLayer, GAlignCenter);
		layer_add_child(window_layer, bitmap_layer_get_layer(newClockBackgroundLayer));
		// Init the text layer used to show the time
		GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
		Layer *backgroundLayer = bitmap_layer_get_layer(newClockBackgroundLayer);
		GRect newFrame = layer_get_bounds(backgroundLayer);
		DEBUG_VERBOSE_LOG("clock bounds: origin (%d, %d), dimensions: (%d, %d)", newFrame.origin.x, newFrame.origin.y, newFrame.size.w, newFrame.size.h);
		newFrame.origin.x += CLOCK_TEXT_X_OFFSET;
		newFrame.origin.y += CLOCK_TEXT_Y_OFFSET;
		newClockLayer = text_layer_create(newFrame);
		text_layer_set_text_color(newClockLayer, GColorWhite);
		text_layer_set_background_color(newClockLayer, GColorClear);
		text_layer_set_font(newClockLayer, font);
		layer_add_child(backgroundLayer, text_layer_get_layer(newClockLayer));
		newClockLayerInitialized = true;
		UpdateNewClock();
	}
	layer_add_child(window_layer, bitmap_layer_get_layer(newClockBackgroundLayer));
}

void FreeClockLayer(void)
{
	bitmap_layer_destroy(newClockBackgroundLayer);
	gbitmap_destroy(newClockBackgroundImage);
	text_layer_destroy(newClockLayer);
}

