#pragma once

void SetLargeImage(int resourceId);
void RemoveLargeImageLayer(void);
void CleanupLargeImageLayer(void);
void InitializeLargeImageLayer(Window *window);
void TriggerLargeImage(int resourceId, bool forceBacklight);
