#pragma once

typedef struct ProgressBar ProgressBar;

typedef enum
{
	FILL_UP,
	FILL_RIGHT,
	FILL_DOWN,
	FILL_LEFT
} FillDirection;


ProgressBar *CreateProgressBar(int *current, int *max, FillDirection fillDirection, GRect frame, GColor fillColor, int iconId);
void ProgressBarUpdateProc(struct Layer *layer, GContext *ctx);
void InitializeProgressBar(ProgressBar *progressBar, Window *window);
void FreeProgressBar(ProgressBar *progressBar);
void RemoveProgressBar(ProgressBar *progressBar);
void ShowProgressBar(ProgressBar *progressBar);
void HideProgressBar(ProgressBar *progressBar);
void MarkProgressBarDirty(ProgressBar *progressBar);
