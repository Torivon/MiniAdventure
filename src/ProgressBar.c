#include <pebble.h>
#include "ProgressBar.h"
#include "Utils.h"

typedef struct ProgressBar
{
	uint16_t *current;
	uint16_t *max;
	FillDirection fillDirection;
	GRect frame;
	GColor fillColor;
	int iconId;
	
	bool initialized;
	
	Layer *layer;
} ProgressBar;



ProgressBar *CreateProgressBar(uint16_t *current, uint16_t *max, FillDirection fillDirection, GRect *frame, GColor fillColor, int iconId)
{
	ProgressBar *bar = calloc(sizeof(ProgressBar), 1);
	
	bar->current = current;
	bar->max = max;
	bar->fillDirection = fillDirection;
	bar->frame = *frame;
	bar->iconId = iconId;
	bar->fillColor = fillColor;
	
	bar->initialized = false;
	return bar;
}

void ProgressBarUpdateProc(struct Layer *layer, GContext *ctx)
{
	GRect bounds = layer_get_bounds(layer);
	void **dataPointer = layer_get_data(layer);
	ProgressBar *progressBar = *dataPointer;
	DrawContentFrame(ctx, &bounds);

	GRect innerFrame = bounds;
	
	innerFrame.origin.x += 2;
	innerFrame.origin.y += 2;
	innerFrame.size.w -= 4;
	innerFrame.size.h -= 4;
	
	if(progressBar->fillDirection == FILL_UP || progressBar->fillDirection == FILL_DOWN)
	{
		int newHeight = innerFrame.size.h * (*(progressBar->current)) / (*(progressBar->max));
        if(newHeight < 0)
            newHeight = 0;
        if(newHeight >= innerFrame.size.h)
            newHeight = innerFrame.size.h;
		if(progressBar->fillDirection == FILL_UP)
		{
			innerFrame.origin.y += innerFrame.size.h - newHeight;
		}
		innerFrame.size.h = newHeight;
	}
	
	if(progressBar->fillDirection == FILL_RIGHT || progressBar->fillDirection == FILL_LEFT)
	{
		int newWidth = innerFrame.size.w * (*(progressBar->current)) / (*(progressBar->max));
        if(newWidth < 0)
            newWidth = 0;
        if(newWidth >= innerFrame.size.w)
            newWidth = innerFrame.size.w;
		if(progressBar->fillDirection == FILL_LEFT)
		{
			innerFrame.origin.x += innerFrame.size.w - newWidth;
		}
		innerFrame.size.w = newWidth;
	}

#if defined(PBL_COLOR)
	graphics_context_set_fill_color(ctx, progressBar->fillColor);
#else
	graphics_context_set_fill_color(ctx, GColorWhite);
#endif
	graphics_fill_rect(ctx, innerFrame, 0, GCornerNone);	
}

void InitializeProgressBar(ProgressBar *progressBar, Window *window)
{
	if(!progressBar->initialized)
	{
		progressBar->layer = layer_create_with_data(progressBar->frame, sizeof(ProgressBar*));
		void **dataPointer = layer_get_data(progressBar->layer);
		*dataPointer = progressBar;
		layer_set_update_proc(progressBar->layer, ProgressBarUpdateProc);
		progressBar->initialized = true;
	}
	
	Layer *windowLayer = window_get_root_layer(window);
	layer_add_child(windowLayer, progressBar->layer);	
}

void RemoveProgressBar(ProgressBar *progressBar)
{
	if(!progressBar || !progressBar->initialized)
		return;
	
	layer_remove_from_parent(progressBar->layer);
}

void FreeProgressBar(ProgressBar *progressBar)
{
    if(!progressBar)
        return;
    
	if(progressBar->initialized)
	{
		layer_destroy(progressBar->layer);
	}
	
	free(progressBar);
}

void ShowProgressBar(ProgressBar *progressBar)
{
	if(!progressBar || !progressBar->initialized)
		return;
	
	if(!layer_get_hidden(progressBar->layer))
		return;
	
    ShowLayer(progressBar->layer);
}

void HideProgressBar(ProgressBar *progressBar)
{
	if(!progressBar || !progressBar->initialized)
		return;
	
	if(layer_get_hidden(progressBar->layer))
		return;
	
    HideLayer(progressBar->layer);
}

void MarkProgressBarDirty(ProgressBar *progressBar)
{
	if(!progressBar || !progressBar->initialized)
		return;

	layer_mark_dirty(progressBar->layer);
}
