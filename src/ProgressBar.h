#pragma once

typedef struct ProgressBar ProgressBar;

typedef enum
{
	FILL_UP,
	FILL_RIGHT,
	FILL_DOWN,
	FILL_LEFT
} FillDirection;


ProgressBar *ProgressBar_Create(uint16_t *current, uint16_t *max, FillDirection fillDirection, GRect *frame, GColor fillColor, int iconId);
void ProgressBar_Initialize(ProgressBar *progressBar, struct Layer *parentLayer);
void ProgressBar_Free(ProgressBar *progressBar);
void ProgressBar_Remove(ProgressBar *progressBar);
void ProgressBar_Show(ProgressBar *progressBar);
void ProgressBar_Hide(ProgressBar *progressBar);
void ProgressBar_MarkDirty(ProgressBar *progressBar);
