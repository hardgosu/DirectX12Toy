#pragma once
/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _DISPLAY_ANIMATION_H
#define _DISPLAY_ANIMATION_H


#include "DisplayCommon.h"



void DisplayAnimation(FbxScene *pfbxScene, std::vector<XMFLOAT4X4> boneOffsets, std::vector<int> boneIndexToParentIndex, std::unordered_map<std::string, AnimationClip> animations);
int GetAnimationLayerCurveNodes(FbxAnimLayer *pfbxAnimationLayer, FbxNode *pfbxNode);
#endif // #ifndef _DISPLAY_ANIMATION_H


