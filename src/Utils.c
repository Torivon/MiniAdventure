#include "pebble.h"

#include "Logging.h"
#include "MiniAdventure.h"
#include "Utils.h"

// Right justified
void IntToString(char *buffer, size_t bufferSize, int value)
{
	int i = bufferSize - 1;
	int digit;
	int temp = value >= 0 ? value : 0;

	DEBUG_VERBOSE_LOG("IntToString(%d)", value);
	
	do
	{
		digit = temp % 10;
		buffer[i] = '0' + digit;
		temp /= 10;
		--i;
	} while(temp);

	while(i >= 0)
	{
#if PAD_WITH_SPACES
		buffer[i] = ' ';
#else
		buffer[i] = '0';
#endif
		--i;
	}
}

void SeedRandom(void)
{
	time_t now = time(NULL);
	srand(now);
}

void IntToPercent(char *buffer, size_t bufferSize, int value)
{
	buffer[bufferSize - 1] = '%';
	IntToString(buffer, bufferSize - 1, value);
}

// Returns an integer in the range [0,max)
uint16_t Random(uint16_t max)
{
	int result = Random_inline(max);
	DEBUG_VERBOSE_LOG("Random(%d)=%d", max, result);
	return result;
}

void DrawContentFrame(GContext * ctx, GRect *rect)
{
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, *rect, 2, GCornersAll);
#if defined(PBL_COLOR)
	graphics_context_set_fill_color(ctx, GColorBlue);
#else
	graphics_context_set_fill_color(ctx, GColorBlack);
#endif
	GRect inner_frame = *rect;
	inner_frame.origin.x += 2;
	inner_frame.origin.y += 2;
	inner_frame.size.w -= 4;
	inner_frame.size.h -= 4;
	graphics_fill_rect(ctx, inner_frame, 0, GCornerNone);	
}
