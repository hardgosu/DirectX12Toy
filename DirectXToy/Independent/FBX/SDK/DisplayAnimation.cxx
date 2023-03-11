/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "DisplayCommon.h"
#include "DisplayAnimation.h"
#include "LoadM3d.h"

using namespace DirectX;

#if defined (FBXSDK_ENV_MAC)
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

int GetAnimationCurves(FbxAnimLayer *pfbxAnimationLayer, FbxNode *pfbxNode)
{
	int nAnimationCurves = 0;

	FbxAnimCurve *pfbxAnimationCurve = NULL;
	if (pfbxAnimationCurve = pfbxNode->LclTranslation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_X)) nAnimationCurves++;
	if (pfbxAnimationCurve = pfbxNode->LclTranslation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Y)) nAnimationCurves++;
	if (pfbxAnimationCurve = pfbxNode->LclTranslation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Z)) nAnimationCurves++;
    if (pfbxAnimationCurve = pfbxNode->LclRotation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_X)) nAnimationCurves++;
	if (pfbxAnimationCurve = pfbxNode->LclRotation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Y)) nAnimationCurves++;
	if (pfbxAnimationCurve = pfbxNode->LclRotation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Z)) nAnimationCurves++;
    if (pfbxAnimationCurve = pfbxNode->LclScaling.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_X)) nAnimationCurves++;
	if (pfbxAnimationCurve = pfbxNode->LclScaling.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Y)) nAnimationCurves++;
	if (pfbxAnimationCurve = pfbxNode->LclScaling.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Z)) nAnimationCurves++;

	return(nAnimationCurves);
}

int GetAnimationLayerCurveNodes(FbxAnimLayer *pfbxAnimationLayer, FbxNode *pfbxNode)
{
	int nAnimationCurveNodes = 0;
	if (GetAnimationCurves(pfbxAnimationLayer, pfbxNode) > 0) nAnimationCurveNodes = 1;

	for (int i = 0; i < pfbxNode->GetChildCount(); i++)
    {
        nAnimationCurveNodes += GetAnimationLayerCurveNodes(pfbxAnimationLayer, pfbxNode->GetChild(i));
    }

	return(nAnimationCurveNodes);
}

void DisplayCurveKeys(char *pHeader, FbxAnimCurve *pfbxAnimationCurve, char *pSuffix, int nTabIndents, bool bRotationAngle)
{
    int nKeys = pfbxAnimationCurve->KeyGetCount();
	DisplayInt(pHeader, nKeys, " ", nTabIndents);

    for (int i = 0; i < nKeys; i++)
    {
		FbxTime fbxKeyTime = pfbxAnimationCurve->KeyGetTime(i);
		float fkeyTime = (float)fbxKeyTime.GetSecondDouble();
		DisplayFloat("", fkeyTime, " ");
    }
    for (int i = 0; i < nKeys; i++)
    {
		float fKeyValue = static_cast<float>(pfbxAnimationCurve->KeyGetValue(i));
		if (bRotationAngle) fKeyValue = (3.1415926f / 180.f) * fKeyValue;
		DisplayFloat("", fKeyValue, " ");
    }

	WriteStringToFile("\n");
}

void DisplayChannels(FbxAnimLayer *pfbxAnimationLayer, FbxNode *pfbxNode, int *pnCurveNode, int nTabIndents)
{
	if (GetAnimationCurves(pfbxAnimationLayer, pfbxNode) > 0)
	{
		DisplayIntString("<AnimationCurve>: ", (*pnCurveNode)++, ReplaceBlank(pfbxNode->GetName(), '_'), "\n", nTabIndents);

		FbxAnimCurve *pfbxAnimationCurve = NULL;

		//FbxAnimCurveNode *pfbxAnimCurveNode = pfbxNode->LclTranslation.GetCurveNode(pfbxAnimationLayer);
		//int nChannels = pfbxAnimCurveNode->GetChannelsCount();
		//int nCurves = pfbxAnimCurveNode->GetCurveCount(0);

		pfbxAnimationCurve = pfbxNode->LclTranslation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_X); //"X"
		if(pfbxAnimationCurve) DisplayCurveKeys("<TX>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, false);

		pfbxAnimationCurve = pfbxNode->LclTranslation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if(pfbxAnimationCurve) DisplayCurveKeys("<TY>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, false);

		pfbxAnimationCurve = pfbxNode->LclTranslation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if(pfbxAnimationCurve) DisplayCurveKeys("<TZ>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, false);

		pfbxAnimationCurve = pfbxNode->LclRotation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if(pfbxAnimationCurve) DisplayCurveKeys("<RX>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, true);

		pfbxAnimationCurve = pfbxNode->LclRotation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if(pfbxAnimationCurve) DisplayCurveKeys("<RY>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, true);

		pfbxAnimationCurve = pfbxNode->LclRotation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if(pfbxAnimationCurve) DisplayCurveKeys("<RZ>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, true);

		pfbxAnimationCurve = pfbxNode->LclScaling.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if(pfbxAnimationCurve) DisplayCurveKeys("<SX>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, false);

		pfbxAnimationCurve = pfbxNode->LclScaling.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if(pfbxAnimationCurve) DisplayCurveKeys("<SY>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, false);

		pfbxAnimationCurve = pfbxNode->LclScaling.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if(pfbxAnimationCurve) DisplayCurveKeys("<SZ>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, false);

		DisplayString("</AnimationCurve>", "", "\n", nTabIndents);
	}
}

void DisplayAnimation(FbxAnimLayer *pfbxAnimationLayer, FbxNode *pfbxNode, int *pnCurveNode, int nTabIndents)
{
    DisplayChannels(pfbxAnimationLayer, pfbxNode, pnCurveNode, nTabIndents);

    for (int i = 0; i < pfbxNode->GetChildCount(); i++)
    {
        DisplayAnimation(pfbxAnimationLayer, pfbxNode->GetChild(i), pnCurveNode, nTabIndents);
    }
}

void DisplayAnimation(FbxAnimStack *pfbxAnimStack, FbxNode *pfbxNode, int nTabIndents)
{
    int nAnimationLayers = pfbxAnimStack->GetMemberCount<FbxAnimLayer>();
	DisplayInt("<AnimationLayers>: ", nAnimationLayers, "\n", nTabIndents);

    for (int i = 0; i < nAnimationLayers; i++)
    {
        FbxAnimLayer *pfbxAnimationLayer = pfbxAnimStack->GetMember<FbxAnimLayer>(i);
		int nLayerCurveNodes = GetAnimationLayerCurveNodes(pfbxAnimationLayer, pfbxNode);

		int nCurveNode = 0;
		DisplayIntFloat("<AnimationLayer>: ", i, nLayerCurveNodes, float(pfbxAnimationLayer->Weight) / 100.0f, "\n", nTabIndents + 1);
        DisplayAnimation(pfbxAnimationLayer, pfbxNode, &nCurveNode, nTabIndents + 2);
		DisplayString("</AnimationLayer>", "", "\n", nTabIndents + 1);
	}

	DisplayString("</AnimationLayers>", "", "\n", nTabIndents);
}

void DisplayAnimation(FbxScene *pfbxScene, std::vector<XMFLOAT4X4> boneOffsets, std::vector<int> boneIndexToParentIndex, std::unordered_map<std::string, AnimationClip> animations)
{
	int nAnimationStacks = pfbxScene->GetSrcObjectCount<FbxAnimStack>();
	DisplayInt("<AnimationSets>: ", nAnimationStacks, "\n", 1);

    for (int i = 0; i < nAnimationStacks; i++)
    {
		FbxAnimStack *pfbxAnimStack = pfbxScene->GetSrcObject<FbxAnimStack>(i);
		DisplayIntString("<AnimationSet>: ", i, ReplaceBlank(pfbxAnimStack->GetName(), '_'), " ", 2);
		FbxTime fbxTimeStart = pfbxAnimStack->LocalStart;
		FbxTime fbxTimeStop = pfbxAnimStack->LocalStop;
		DisplayFloat("", (float)fbxTimeStart.GetSecondDouble(), (float)fbxTimeStop.GetSecondDouble(), "\n");

        DisplayAnimation(pfbxAnimStack, pfbxScene->GetRootNode(), 3);
		DisplayString("</AnimationSet>", "", "\n", 2);
    }

	DisplayString("</AnimationSets>", "", "\n", 1);
}

