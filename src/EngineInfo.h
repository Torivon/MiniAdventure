#pragma once


typedef struct EngineInfo
{
    uint16_t titleImage;
    uint16_t rightArrowImage;
    uint16_t leftArrowImage;
    uint16_t restImage;
    uint16_t battleFloorImage;
    uint16_t engineRepositoryImage;
    uint16_t defaultImage;
    uint16_t tutorialDialog;
    uint16_t gameOverDialog;
    uint16_t battleWinDialog;
    uint16_t levelUpDialog;
    uint16_t engineCreditsDialog;
    uint16_t resetPromptDialog;
    uint16_t exitPromptDialog;
} EngineInfo;

ResHandle EngineInfo_GetResHandle(void);
EngineInfo *EngineInfo_GetInfo(void);

void EngineInfo_QueueWinDialog(void);
void EngineInfo_QueueLevelUpDialog(void);
void EngineInfo_QueueResetDialog(void);
