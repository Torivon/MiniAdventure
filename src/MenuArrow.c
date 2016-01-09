#include <pebble.h>
#include "MenuArrow.h"
#include "MiniAdventure.h"
#include "Menu.h"


//******* MENU ARROW *********//

static BitmapLayer *menuArrowLayer;
static GBitmap *rightArrowImage;
static GBitmap *leftArrowImage;
#define ARROW_DIMENSION 10
#define ARROW_OFFSET 5
#if defined(PBL_RECT)
static GRect menuArrowInactiveFrame = {.origin = {.x = 144 - ARROW_DIMENSION - ARROW_OFFSET, .y = 168 / 2 - ARROW_DIMENSION / 2}, .size = {.w = ARROW_DIMENSION, .h = ARROW_DIMENSION}};
static GRect menuArrowActiveFrame = {.origin = {.x = ARROW_OFFSET, .y = 168 / 4 - ARROW_DIMENSION / 2}, .size = {.w = ARROW_DIMENSION, .h = ARROW_DIMENSION}};
#elif defined(PBL_ROUND)
static GRect menuArrowInactiveFrame = {.origin = {.x = 180 - ARROW_DIMENSION - ARROW_OFFSET, .y = 180 / 2 - ARROW_DIMENSION / 2}, .size = {.w = ARROW_DIMENSION, .h = ARROW_DIMENSION}};
static GRect menuArrowActiveFrame = {.origin = {.x = ARROW_OFFSET, .y = 180 / 2 - ARROW_DIMENSION / 2}, .size = {.w = ARROW_DIMENSION, .h = ARROW_DIMENSION}};
#endif
static bool menuArrowInitialized = false;
static bool menuArrowRight = false;
static PropertyAnimation *menuArrowActivateAnimation = NULL;
static PropertyAnimation *menuArrowInactivateAnimation = NULL;
static bool arrowAnimating = false;

void SetMenuArrowRight(void)
{
	if(!menuArrowInitialized || menuArrowRight)
		return;

	bitmap_layer_set_bitmap(menuArrowLayer, rightArrowImage);
	menuArrowRight = true;
}

void SetMenuArrowLeft(void)
{
	if(!menuArrowInitialized || !menuArrowRight)
		return;
	
	bitmap_layer_set_bitmap(menuArrowLayer, leftArrowImage);
	menuArrowRight = false;
}

static void ActivateAnimationStarted(struct Animation *animation, void *context)
{
	SetMenuArrowLeft();
	arrowAnimating = true;
}

static void ActivateAnimationStopped(struct Animation *animation, bool finished, void *context)
{
	arrowAnimating = false;

	if(finished)
		SetMenuArrowRight();
	
#if !defined(PBL_PLATFORM_APLITE)
	menuArrowActivateAnimation = NULL;
#endif
}

static void InactivateAnimationStarted(struct Animation *animation, void *context)
{
	SetMenuArrowRight();
	arrowAnimating = true;
}

static void InactivateAnimationStopped(struct Animation *animation, bool finished, void *context)
{
	arrowAnimating = false;
	
	if(finished)
		SetMenuArrowLeft();

#if !defined(PBL_PLATFORM_APLITE)
	menuArrowInactivateAnimation = NULL;	
#endif
}
	
void ActivateMenuArrow(void)
{
	if(menuArrowActivateAnimation && animation_is_scheduled((Animation*)menuArrowActivateAnimation))
		return;

	if(!menuArrowActivateAnimation)
	{
		menuArrowActivateAnimation = property_animation_create_layer_frame(bitmap_layer_get_layer(menuArrowLayer), NULL, &menuArrowActiveFrame);
		animation_set_duration((Animation*)menuArrowActivateAnimation, MENU_ANIMATION_DURATION);
		animation_set_curve((Animation*)menuArrowActivateAnimation, AnimationCurveLinear);
		AnimationHandlers showHandlers = {.started = ActivateAnimationStarted, .stopped = ActivateAnimationStopped};
		animation_set_handlers((Animation*)menuArrowActivateAnimation, showHandlers, NULL);
	}
	
	if(menuArrowInactivateAnimation)
		animation_unschedule((Animation*) menuArrowInactivateAnimation);
	animation_schedule((Animation*) menuArrowActivateAnimation);
}

void InactivateMenuArrow(void)
{
	if(menuArrowInactivateAnimation && animation_is_scheduled((Animation*)menuArrowInactivateAnimation))
		return;

	if(!menuArrowInactivateAnimation)
	{
		menuArrowInactivateAnimation = property_animation_create_layer_frame(bitmap_layer_get_layer(menuArrowLayer), NULL, &menuArrowInactiveFrame);
		animation_set_duration((Animation*)menuArrowInactivateAnimation, MENU_ANIMATION_DURATION);
		animation_set_curve((Animation*)menuArrowInactivateAnimation, AnimationCurveLinear);
		AnimationHandlers hideHandlers = {.started = InactivateAnimationStarted, .stopped = InactivateAnimationStopped};
		animation_set_handlers((Animation*)menuArrowInactivateAnimation, hideHandlers, NULL);
	}
	
	if(menuArrowActivateAnimation)
		animation_unschedule((Animation*) menuArrowActivateAnimation);
	animation_schedule((Animation*) menuArrowInactivateAnimation);
}

void HideMenuArrow(void)
{
	if(menuArrowInitialized)
	{
		layer_set_hidden((Layer*)menuArrowLayer, true);
	}
}

void ShowMenuArrow(void)
{
	if(menuArrowInitialized)
	{
		layer_set_hidden((Layer*)menuArrowLayer, false);
	}	
}

void RemoveMenuArrowLayer(void)
{
	if(!menuArrowInitialized)
		return;

	layer_remove_from_parent((Layer*)menuArrowLayer);
}

void InitializeMenuArrowLayer(Window *window)
{
	Layer *window_layer = window_get_root_layer(window);
	if(!menuArrowInitialized)
	{
		rightArrowImage = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RIGHT_ARROW);
		leftArrowImage = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LEFT_ARROW);
		menuArrowLayer = bitmap_layer_create(menuArrowInactiveFrame);
		bitmap_layer_set_bitmap(menuArrowLayer, leftArrowImage);
		menuArrowRight = false;
		bitmap_layer_set_alignment(menuArrowLayer, GAlignCenter);

#if defined(PBL_PLATFORM_APLITE)
		bitmap_layer_set_compositing_mode(menuArrowLayer, GCompOpOr);		
#endif

#if defined(PBL_COLOR)
		bitmap_layer_set_compositing_mode(menuArrowLayer, GCompOpSet);
#endif
		menuArrowInitialized = true;
	}
	layer_add_child(window_layer, (Layer*)menuArrowLayer);
}

void FreeMenuArrowLayer()
{
	bitmap_layer_destroy(menuArrowLayer);
	gbitmap_destroy(rightArrowImage);
	gbitmap_destroy(leftArrowImage);	
	if(menuArrowActivateAnimation)
		property_animation_destroy(menuArrowActivateAnimation);
	if(menuArrowInactivateAnimation)
		property_animation_destroy(menuArrowInactivateAnimation);
}
