//
//  BattleStatusFrame.h
//  
//
//  Created by Jonathan Panttaja on 3/3/16.
//
//

#pragma once

typedef struct BattleStatusFrame BattleStatusFrame;
typedef struct BattleActor BattleActor;

BattleStatusFrame *BattleStatusFrame_Create(BattleActor *actor, GRect *frame, GColor fillColor, GColor healthFillColor, GColor timeFillColor);
void BattleStatusFrame_Free(BattleStatusFrame *statusFrame);
void BattleStatusFrame_Show(BattleStatusFrame *statusFrame);
void BattleStatusFrame_Hide(BattleStatusFrame *statusFrame);

void BattleStatusFrame_Initialize(BattleStatusFrame *statusFrame, Window *window);
void BattleStatusFrame_Remove(BattleStatusFrame *statusFrame);

void BattleStatusFrame_MarkDirty(BattleStatusFrame *statusFrame);
