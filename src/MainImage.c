#include <pebble.h>
#include "MainImage.h"
#include "MiniAdventure.h"

#define MAIN_IMAGE_LAYER_X 50
#define MAIN_IMAGE_LAYER_Y 20
#define MAIN_IMAGE_LAYER_W 100
#define MAIN_IMAGE_LAYER_H 100

#define INTERNAL_IMAGE_OFFSET 2

static GBitmap *mainImageBackgroundFrame = NULL;
static GBitmap *foregroundImage = NULL;
static GBitmap *backgroundImage = NULL;

static Layer *mainImageTopLayer = NULL;
static BitmapLayer *mainImageBackgroundLayer = NULL;
static BitmapLayer *foregroundImageLayer = NULL;
static BitmapLayer *backgroundImageLayer = NULL;

static GRect mainImagePosition = {.origin = {.x = MAIN_IMAGE_LAYER_X, .y = MAIN_IMAGE_LAYER_Y}, .size = {.w = MAIN_IMAGE_LAYER_W, .h = MAIN_IMAGE_LAYER_H}};

static bool mainImageInitialized;

static int foregroundResourceId = -1;
static int backgroundResourceId = -1;

void InitializeMainImageLayer(Window *window)
{
	if(!mainImageInitialized)
	{
		mainImageBackgroundFrame = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MAIN_FRAME);
		GRect frameBounds = gbitmap_get_bounds(mainImageBackgroundFrame);
		mainImagePosition.size = frameBounds.size;
		GRect screen_bounds = layer_get_bounds(window_get_root_layer(window));
		mainImagePosition.origin.x = screen_bounds.size.w / 2 - mainImagePosition.size.w / 2;
		mainImagePosition.origin.y = screen_bounds.size.h / 2 - mainImagePosition.size.h / 2;
		mainImageTopLayer = layer_create(mainImagePosition);
		mainImageBackgroundLayer = bitmap_layer_create(layer_get_bounds(mainImageTopLayer));
		bitmap_layer_set_bitmap(mainImageBackgroundLayer, mainImageBackgroundFrame);
		bitmap_layer_set_alignment(mainImageBackgroundLayer, GAlignCenter);
		layer_add_child(mainImageTopLayer, bitmap_layer_get_layer(mainImageBackgroundLayer));
		
		GRect image_bounds = layer_get_bounds(mainImageTopLayer);
		image_bounds.origin.x += INTERNAL_IMAGE_OFFSET;
		image_bounds.origin.y += INTERNAL_IMAGE_OFFSET;
		image_bounds.size.w -= 2 * INTERNAL_IMAGE_OFFSET;
		image_bounds.size.h -= 2 * INTERNAL_IMAGE_OFFSET;

		backgroundImageLayer = bitmap_layer_create(image_bounds);
		backgroundResourceId = RESOURCE_ID_IMAGE_BATTLE_FLOOR;
		backgroundImage = gbitmap_create_with_resource(backgroundResourceId);
		bitmap_layer_set_bitmap(backgroundImageLayer, backgroundImage);
		bitmap_layer_set_alignment(backgroundImageLayer, GAlignCenter);
		layer_add_child(mainImageTopLayer, bitmap_layer_get_layer(backgroundImageLayer));
		
		foregroundImageLayer = bitmap_layer_create(image_bounds);
		foregroundResourceId = RESOURCE_ID_IMAGE_TITLE;
		foregroundImage = gbitmap_create_with_resource(foregroundResourceId);
		bitmap_layer_set_bitmap(foregroundImageLayer, foregroundImage);
		bitmap_layer_set_alignment(foregroundImageLayer, GAlignCenter);
#if defined(PBL_COLOR)
		bitmap_layer_set_compositing_mode(foregroundImageLayer, GCompOpSet);
#endif

		layer_add_child(mainImageTopLayer, bitmap_layer_get_layer(foregroundImageLayer));

		mainImageInitialized = true;
		SetMainImageVisibility(false, false, false);
	}
	Layer *window_layer = window_get_root_layer(window);
	layer_add_child(window_layer, mainImageTopLayer);
}

void SetForegroundImage(int resourceId)
{
	if(!mainImageInitialized)
		return;
	
	if(resourceId == foregroundResourceId)
		return;
	
	foregroundResourceId = resourceId;
	gbitmap_destroy(foregroundImage);
	foregroundImage = gbitmap_create_with_resource(foregroundResourceId);
	bitmap_layer_set_bitmap(foregroundImageLayer, foregroundImage);
}

void SetBackgroundImage(int resourceId)
{
	if(!mainImageInitialized)
		return;
	
	if(resourceId == backgroundResourceId)
		return;
	
	backgroundResourceId = resourceId;
	gbitmap_destroy(backgroundImage);
	backgroundImage = gbitmap_create_with_resource(backgroundResourceId);
	bitmap_layer_set_bitmap(backgroundImageLayer, backgroundImage);	
}

void SetMainImageVisibility(bool topLevelVisible, bool foregroundVisible, bool backgroundVisible)
{
	if(!mainImageInitialized)
		return;
	
	layer_set_hidden(mainImageTopLayer, !topLevelVisible);
	layer_set_hidden(bitmap_layer_get_layer(backgroundImageLayer), !backgroundVisible);
	layer_set_hidden(bitmap_layer_get_layer(foregroundImageLayer), !foregroundVisible);
}

void RemoveMainImageLayer(void)
{
	if(!mainImageInitialized)
		return;
	
	layer_remove_from_parent(mainImageTopLayer);
}

void CleanupMainImageLayer(void)
{
	if(mainImageInitialized)
	{
		layer_destroy(mainImageTopLayer);
		bitmap_layer_destroy(mainImageBackgroundLayer);
		bitmap_layer_destroy(foregroundImageLayer);
		bitmap_layer_destroy(backgroundImageLayer);
		gbitmap_destroy(mainImageBackgroundFrame);
		gbitmap_destroy(foregroundImage);
		gbitmap_destroy(backgroundImage);
		mainImageInitialized = false;
	}
}
