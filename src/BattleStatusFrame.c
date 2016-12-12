//
//  BattleStatusFrame.c
//  
//
//  Created by Jonathan Panttaja on 3/3/16.
//
//
#include <pebble.h>
#include "AutoSizeConstants.h"
#include "BattleStatusFrame.h"
#include "EngineInfo.h"
#include "ImageMap.h"
#include "MiniAdventure.h"
#include "ProgressBar.h"
#include "Skills.h"
#include "Utils.h"

static uint16_t maxTimeCount = MAX_TIME_COUNT;

typedef struct StatusEffectIcon
{
    GBitmap *bitmap;
} StatusEffectIcon;

typedef struct BattleStatusFrame
{
    BattleActor *actor;
    ProgressBar *healthBar;
    ProgressBar *timeBar;
    StatusEffectIcon effectIcons[STATUS_EFFECT_COUNT];
    GRect frame;
    GColor fillColor;
    GColor healthFillColor;
    GColor timeFillColor;
    
    Layer *mainLayer;
    bool initialized;
} BattleStatusFrame;

BattleStatusFrame *BattleStatusFrame_Create(BattleActor *actor, GRect *frame, GColor fillColor, GColor healthFillColor, GColor timeFillColor)
{
    BattleStatusFrame *statusFrame = calloc(sizeof(BattleStatusFrame), 1);
    statusFrame->actor = actor;
    statusFrame->frame = *frame;
    statusFrame->fillColor = fillColor;
    statusFrame->healthFillColor = healthFillColor;
    statusFrame->timeFillColor = timeFillColor;
    return statusFrame;
}

void BattleStatusFrame_Free(BattleStatusFrame *statusFrame)
{
    if(statusFrame)
    {
        if(statusFrame->initialized)
        {
            for(int i = 0; i < MAX_STATUS_ICONS; ++i)
            {
                if(statusFrame->effectIcons[i].bitmap)
                {
                    gbitmap_destroy(statusFrame->effectIcons[i].bitmap);
                    statusFrame->effectIcons[i].bitmap = NULL;
                }
            }
            
            layer_destroy(statusFrame->mainLayer);
            ProgressBar_Free(statusFrame->healthBar);
            ProgressBar_Free(statusFrame->timeBar);
        }
        free(statusFrame);
    }
}

void BattleStatusFrame_Show(BattleStatusFrame *statusFrame)
{
    if(!statusFrame || !statusFrame->initialized)
        return;

    layer_set_hidden(statusFrame->mainLayer, false);
}

void BattleStatusFrame_Hide(BattleStatusFrame *statusFrame)
{
    if(!statusFrame || !statusFrame->initialized)
        return;

    layer_set_hidden(statusFrame->mainLayer, true);
}

void BattleStatusFrame_UpdateProc(struct Layer *layer, GContext *ctx)
{
    GRect bounds = layer_get_bounds(layer);
    void **dataPointer = layer_get_data(layer);
    BattleStatusFrame *statusFrame = *dataPointer;
    DrawContentFrame(ctx, &bounds, statusFrame->fillColor);
    
    int position = 0;
    
    for(int i = 0; i < STATUS_EFFECT_COUNT; ++i)
    {
        StatusEffectIcon *icon = &statusFrame->effectIcons[i];
        if(statusFrame->actor->statusEffectDurations[i] > 0)
        {
            GRect image_bounds = { .origin = {.x = 4 + position * (STATUS_EFFECT_ICON_SIZE + 1), .y = STATUS_EFFECT_ICON_Y}, .size = {.w = STATUS_EFFECT_ICON_SIZE, .h = STATUS_EFFECT_ICON_SIZE}};
            
            if(icon->bitmap == NULL)
            {
                uint16_t iconIndex = EngineInfo_GetInfo()->statusEffectIcons[i];
                icon->bitmap = gbitmap_create_with_resource(ImageMap_GetIdByIndex(iconIndex));
            }
            
            if(image_bounds.origin.x + image_bounds.size.w < bounds.size.w - 2)
                graphics_draw_bitmap_in_rect(ctx, icon->bitmap, image_bounds);
            position++;
        }
    }
}

void BattleStatusFrame_Initialize(BattleStatusFrame *statusFrame, Window *window)
{
    if(!statusFrame->initialized)
    {
        GRect relativeHealthFrame = RELATIVE_HEALTH_FRAME;
        GRect relativeTimeFrame = RELATIVE_TIME_FRAME;
        statusFrame->mainLayer = layer_create_with_data(statusFrame->frame, sizeof(BattleStatusFrame*));
        void **dataPointer = layer_get_data(statusFrame->mainLayer);
        *dataPointer = statusFrame;
        layer_set_update_proc(statusFrame->mainLayer, BattleStatusFrame_UpdateProc);
        
        BattleActor *actor = statusFrame->actor;
        statusFrame->healthBar = ProgressBar_Create(&actor->currentHealth, &actor->maxHealth, FILL_RIGHT, &relativeHealthFrame, statusFrame->healthFillColor, -1);
        ProgressBar_Initialize(statusFrame->healthBar, statusFrame->mainLayer);
        statusFrame->timeBar = ProgressBar_Create(&actor->currentTime, &maxTimeCount, FILL_RIGHT, &relativeTimeFrame, statusFrame->timeFillColor, -1);
        ProgressBar_Initialize(statusFrame->timeBar, statusFrame->mainLayer);
        
        statusFrame->initialized = true;
    }
    
    Layer *windowLayer = window_get_root_layer(window);
    layer_add_child(windowLayer, statusFrame->mainLayer);
    
}

void BattleStatusFrame_Remove(BattleStatusFrame *statusFrame)
{
    if(!statusFrame || !statusFrame->initialized)
        return;
    
    layer_remove_from_parent(statusFrame->mainLayer);
}

void BattleStatusFrame_MarkDirty(BattleStatusFrame *statusFrame)
{
    if(!statusFrame || !statusFrame->initialized)
        return;
    
    layer_mark_dirty(statusFrame->mainLayer);
    ProgressBar_MarkDirty(statusFrame->healthBar);
    ProgressBar_MarkDirty(statusFrame->timeBar);
}

