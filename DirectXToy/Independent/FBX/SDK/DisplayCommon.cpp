/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "DisplayCommon.h"
#if defined (FBXSDK_ENV_MAC)
// disable the “format not a string literal and no format arguments?warning since
// the PrintToFile calls made here are all valid calls and there is no secuity risk
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

void DisplayMetaDataConnections(FbxObject *pfbxObject)
{
	int nbMetaData = pfbxObject->GetSrcObjectCount<FbxObjectMetaData>();
    if (nbMetaData > 0)
        DisplayString("    MetaData connections ");

    for (int i = 0; i < nbMetaData; i++)
    {
        FbxObjectMetaData* metaData = pfbxObject->GetSrcObject<FbxObjectMetaData>(i);
        DisplayString("        Name: ", (char*)metaData->GetName());
    }
}

FbxString gfbxString;
char *ReplaceBlank(const char *value, const char chReplace)
{
	gfbxString = value;
	gfbxString.ReplaceAll(' ', chReplace);
	return(gfbxString.Buffer());
}

void DisplayString(char *pHeader, const char *value, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

	fbxString = pHeader;
    fbxString += value;
    fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayIntString(char *pHeader, int iValue, const char *sValue, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

	fbxString = pHeader;
	fbxString += iValue;
	fbxString += " ";
	fbxString += sValue;
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayBool(char *pHeader, bool value, char *pSuffix, int nTabIndents)
{
    FbxString fbxString;

    fbxString = pHeader;
    fbxString += value ? "true" : "false";
    fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayInt(int value)
{
	FbxString fbxString;

	fbxString += value;
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void DisplayInt(char *pHeader, int value, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

    fbxString = pHeader;
    fbxString += value;
    fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayInt(char *pHeader, int value1, int value2, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

	fbxString = pHeader;
	fbxString += value1;
	fbxString += " ";
	fbxString += value2;
	fbxString += " ";
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayIntFloat(char *pHeader, int value1, int value2, float value3, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

	fbxString = pHeader;
	fbxString += value1;
	fbxString += " ";
	fbxString += value2;
	fbxString += " ";
	fbxString += value3;
	fbxString += " ";
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayInt(char *pHeader, int value1, int value2, int value3, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

	fbxString = pHeader;
	fbxString += value1;
	fbxString += " ";
	fbxString += value2;
	fbxString += " ";
	fbxString += value3;
	fbxString += " ";
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayIntString(char *pHeader, int value1, int value2, int value3, char *pString, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

	fbxString = pHeader;
	fbxString += value1;
	fbxString += " ";
	fbxString += value2;
	fbxString += " ";
	fbxString += value3;
	fbxString += " ";
	fbxString += pString;
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayInt(char *pHeader, int value1, int value2, int value3, int value4, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

	fbxString = pHeader;
	fbxString += value1;
	fbxString += " ";
	fbxString += value2;
	fbxString += " ";
	fbxString += value3;
	fbxString += " ";
	fbxString += value4;
	fbxString += " ";
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayInt(int value1, int value2, int value3, int value4)
{
	FbxString fbxString;

	fbxString += value1;
	fbxString += " ";
	fbxString += value2;
	fbxString += " ";
	fbxString += value3;
	fbxString += " ";
	fbxString += value4;
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void DisplayFloat(float value1, float value2, float value3, float value4)
{
	FbxString fbxString;

	fbxString += value1;
	fbxString += " ";
	fbxString += value2;
	fbxString += " ";
	fbxString += value3;
	fbxString += " ";
	fbxString += value4;
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void DisplayFloat(float value1, float value2)
{
	FbxString fbxString;

	fbxString += value1;
	fbxString += " ";
	fbxString += value2;
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void DisplayDouble(double value)
{
	FbxString fbxString;

	fbxString += value;
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void DisplayFloat(float value)
{
	FbxString fbxString;

	fbxString += value;
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void DisplayDouble(char *pHeader, double value, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;
    FbxString lFloatValue = (float)value;

    lFloatValue = value <= -HUGE_VAL ? "-INFINITY" : lFloatValue.Buffer();
    lFloatValue = value >=  HUGE_VAL ?  "INFINITY" : lFloatValue.Buffer();

    fbxString = pHeader;
    fbxString += lFloatValue;
    fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayFloat(char *pHeader, float value, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

	fbxString = pHeader;
	fbxString += value;
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayFloat(char *pHeader, float value1, float value2, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

	fbxString = pHeader;
	fbxString += value1;
	fbxString += " ";
	fbxString += value2;
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void Display2DVector(FbxVector2 value)
{
	FbxString fbxString;
	FbxString lFloatValue1 = (float)value[0];
	FbxString lFloatValue2 = (float)value[1];

	fbxString += lFloatValue1;
	fbxString += " ";
	fbxString += lFloatValue2;
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void Display2DVector(char *pHeader, FbxVector2 value, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;
	FbxString lFloatValue1 = (float)value[0];
	FbxString lFloatValue2 = (float)value[1];

	lFloatValue1 = value[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
	lFloatValue1 = value[0] >=  HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
	lFloatValue2 = value[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
	lFloatValue2 = value[1] >=  HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();

	fbxString = pHeader;
	fbxString += lFloatValue1;
	fbxString += " ";
	fbxString += lFloatValue2;
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void Display3DVector(FbxVector4 value)
{
	FbxString fbxString;

	fbxString += (float)value[0];
	fbxString += " ";
	fbxString += (float)value[1];
	fbxString += " ";
	fbxString += (float)value[2];
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void Display3DVector(char *pHeader, FbxVector4 value, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;
	FbxString lFloatValue1 = (float)value[0];
	FbxString lFloatValue2 = (float)value[1];
	FbxString lFloatValue3 = (float)value[2];

	lFloatValue1 = value[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
	lFloatValue1 = value[0] >=  HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
	lFloatValue2 = value[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
	lFloatValue2 = value[1] >=  HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();
	lFloatValue3 = value[2] <= -HUGE_VAL ? "-INFINITY" : lFloatValue3.Buffer();
	lFloatValue3 = value[2] >=  HUGE_VAL ? "INFINITY" : lFloatValue3.Buffer();

	fbxString = pHeader;
	fbxString += lFloatValue1;
	fbxString += " ";
	fbxString += lFloatValue2;
	fbxString += " ";
	fbxString += lFloatValue3;
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void Display4DVector(FbxVector4 value)
{
	FbxString fbxString;

	fbxString += (float)value[0];
	fbxString += " ";
	fbxString += (float)value[1];
	fbxString += " ";
	fbxString += (float)value[2];
	fbxString += " ";
	fbxString += (float)value[3];
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void Display4DVector(char *pHeader, FbxVector4 value, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;
	FbxString lFloatValue1 = (float)value[0];
	FbxString lFloatValue2 = (float)value[1];
	FbxString lFloatValue3 = (float)value[2];
	FbxString lFloatValue4 = (float)value[3];

	lFloatValue1 = value[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
	lFloatValue1 = value[0] >=  HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
	lFloatValue2 = value[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
	lFloatValue2 = value[1] >=  HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();
	lFloatValue3 = value[2] <= -HUGE_VAL ? "-INFINITY" : lFloatValue3.Buffer();
	lFloatValue3 = value[2] >=  HUGE_VAL ? "INFINITY" : lFloatValue3.Buffer();
	lFloatValue4 = value[3] <= -HUGE_VAL ? "-INFINITY" : lFloatValue4.Buffer();
	lFloatValue4 = value[3] >=  HUGE_VAL ? "INFINITY" : lFloatValue4.Buffer();

	fbxString = pHeader;
	fbxString += lFloatValue1;
	fbxString += ", ";
	fbxString += lFloatValue2;
	fbxString += ", ";
	fbxString += lFloatValue3;
	fbxString += ", ";
	fbxString += lFloatValue4;
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayMatrix(FbxAMatrix value)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++) DisplayFloat((float)value[i][j]);
	}
}

void DisplayMatrix(char *pHeader, FbxAMatrix value, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	WriteStringToFile(pHeader);
	DisplayMatrix(value);
	WriteStringToFile(pSuffix);
}

void DisplayColor(char *pHeader, FbxPropertyT<FbxDouble3> value, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

    fbxString = pHeader;
    //fbxString += (float) value.mRed;
    //fbxString += (double)value.GetArrayItem(0);
    fbxString += "(R), ";
    //fbxString += (float) value.mGreen;
    //fbxString += (double)value.GetArrayItem(1);
    fbxString += "(G), ";
    //fbxString += (float) value.mBlue;
    //fbxString += (double)value.GetArrayItem(2);
    fbxString += "(B)";
    fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayColor(FbxColor value)
{
	FbxString fbxString;

	fbxString += (float)value.mRed;
	fbxString += (float)value.mGreen;
	fbxString += (float)value.mBlue;
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void DisplayColor(char *pHeader, FbxColor value, char *pSuffix, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

    fbxString = pHeader;
    fbxString += (float)value.mRed;
    fbxString += "(R), ";
    fbxString += (float)value.mGreen;
    fbxString += "(G), ";
    fbxString += (float)value.mBlue;
    fbxString += "(B)";
    fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

