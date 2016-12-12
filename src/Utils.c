#include <pebble.h>

#include "Logging.h"
#include "MiniAdventure.h"
#include "Utils.h"

void SeedRandom(void)
{
	time_t now = time(NULL);
	srand(now);
}

// Returns an integer in the range [0,max)
uint16_t Random(uint16_t max)
{
	int result = Random_inline(max);
	return result;
}

void DrawContentFrame(GContext * ctx, GRect *rect, GColor fillColor)
{
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, *rect, 2, GCornersAll);
#if defined(PBL_COLOR)
	graphics_context_set_fill_color(ctx, fillColor);
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

void DrawBoundaryArcs(GContext * ctx, GRect *rect)
{
    graphics_context_set_fill_color(ctx, GColorWhite);
    GRect screenBounds = {.origin = {.x = -rect->origin.x, .y = -rect->origin.y}, .size = {.w = 180, .h = 180}};
    graphics_fill_radial(ctx, grect_inset(screenBounds, GEdgeInsets(1, 2, 1, 1)), GOvalScaleModeFillCircle, 4, 0, TRIG_MAX_ANGLE);
}

void ShowLayer(Layer *layer)
{
    layer_set_hidden(layer, false);
}

void HideLayer(Layer *layer)
{
    layer_set_hidden(layer, true);
}
