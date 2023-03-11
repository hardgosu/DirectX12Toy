#pragma once
/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _DISPLAY_COMMON_H
#define _DISPLAY_COMMON_H

#include <fbxsdk.h>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <DirectXMath.h>
#include "..//SkinningHelper.h"

using namespace DirectX;

char* ReplaceBlank(const char *value, const char chReplace);

void DisplayMetaDataConnections(FbxObject *pfbxNode);
void DisplayString(char *pHeader, const char *value  = "", char *pSuffix  = "", int nTabIndents = 0);
void DisplayIntString(char *pHeader, int iValue, const char *sValue, char *pSuffix, int nTabIndents = 0);
void DisplayBool(char *pHeader, bool value, char *pSuffix  = "", int nTabIndents = 0);
void DisplayInt(int value);
void DisplayInt(char *pHeader, int value, char *pSuffix  = "", int nTabIndents = 0);
void DisplayInt(char *pHeader, int value1, int value2, char *pSuffix  = "", int nTabIndents = 0);
void DisplayIntFloat(char *pHeader, int value1, int value2, float value3, char *pSuffix  = "", int nTabIndents = 0);
void DisplayInt(char *pHeader, int value1, int value2, int value3, char *pSuffix  = "", int nTabIndents = 0);
void DisplayIntString(char *pHeader, int value1, int value2, int value3, char *pString, char *pSuffix, int nTabIndents = 0);
void DisplayInt(char *pHeader, int value1, int value2, int value3, int value4, char *pSuffix  = "", int nTabIndents = 0);
void DisplayInt(int value1, int value2, int value3, int value4);
void DisplayFloat(float value1, float value2, float value3, float value4);
void DisplayDouble(double value);
void DisplayFloat(float value);
void DisplayFloat(float value1, float value2);
void DisplayDouble(char *pHeader, double value, char *pSuffix  = "", int nTabIndents = 0);
void DisplayFloat(char *pHeader, float value, char *pSuffix = "", int nTabIndents = 0);
void DisplayFloat(char *pHeader, float value1, float value2, char *pSuffix = "", int nTabIndents = 0);
void Display2DVector(FbxVector2 value);
void Display2DVector(char *pHeader, FbxVector2 value, char *pSuffix  = "", int nTabIndents = 0);
void Display3DVector(FbxVector4 value);
void Display3DVector(char *pHeader, FbxVector4 value, char *pSuffix  = "", int nTabIndents = 0);
void DisplayColor(FbxColor value);
void DisplayColor(char *pHeader, FbxColor value, char *pSuffix  = "", int nTabIndents = 0);
void Display4DVector(FbxVector4 value);
void Display4DVector(char *pHeader, FbxVector4 value, char *pSuffix  = "", int nTabIndents = 0);
void DisplayMatrix(FbxAMatrix value);
void DisplayMatrix(char *pHeader, FbxAMatrix value, char *pSuffix = "", int nTabIndents = 0);

extern void WriteStringToFile(char *pszBuffer);

#endif // #ifndef _DISPLAY_COMMON_H


