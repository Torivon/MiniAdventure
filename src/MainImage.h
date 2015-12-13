#pragma once

void SetForegroundImage(int resourceId);
void SetBackgroundImage(int resourceId);
void SetMainImageVisibility(bool topLevelVisible, bool foregroundVisible, bool backgroundVisible);
void RemoveMainImageLayer(void);
void CleanupMainImageLayer(void);
void InitializeMainImageLayer(Window *window);
