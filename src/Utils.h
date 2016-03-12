#pragma once

#define BIT_FIELD(num) 1 << num

typedef struct Layer Layer;

void SeedRandom(void);

// Returns an integer in the range [0,max)
inline uint16_t Random_inline(uint16_t max)
{
	int result = (uint16_t)(rand() % max);
	return result;
}

uint16_t Random(uint16_t max);

#define BOOL_TO_STR(b) (b) ? "true" : "false"

typedef struct GContext GContext;
typedef struct GRect GRect;

#ifndef BUILD_WORKER_FILES
void DrawContentFrame(GContext *ctx, GRect *rect, GColor fillColor);
#endif
void DrawBoundaryArcs(GContext * ctx, GRect *rect);
void ShowLayer(Layer *layer);
void HideLayer(Layer *layer);
