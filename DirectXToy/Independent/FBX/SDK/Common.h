#pragma once
/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/
#ifndef _COMMON_H
#define _COMMON_H
#pragma comment(lib,"libfbxsdk-md.lib")
#pragma comment(lib,"libxml2-md.lib")
#pragma comment(lib,"zlib-md.lib")

#include <Windows.h>
#include <DirectXMath.h>
#include <iostream>
#include <vector>
#include <unordered_map>

#include <fbxsdk.h>


using namespace DirectX;


struct SkinnedVertex
{
	XMFLOAT3 position_{ 0.0f,0.0f,0.0f };
	XMFLOAT4 color_{ 0.0f,0.0f,1.0f,1.0f };
	XMFLOAT3 normal_{ 0.0f,1.0f,0.0f };
	XMFLOAT2 textureCoordinate_{ 0,0 };
	XMFLOAT3 tangent_{ 1.0f,0.0f,0.0f };
	unsigned char boneIndices_[4]{};
	XMFLOAT4 boneWeights_{ 1.0f,0,0,0 };
};


struct Keyframe
{
	Keyframe();
	~Keyframe();

	float TimePos;
	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;
};

struct BoneAnimation
{
	float GetStartTime()const;
	float GetEndTime()const;

	void Interpolate(float t, DirectX::XMFLOAT4X4& M)const;

	std::vector<Keyframe> Keyframes;
};
struct AnimationClip
{
	float GetClipStartTime()const;
	float GetClipEndTime()const;

	void Interpolate(float t, std::vector<DirectX::XMFLOAT4X4>& boneTransforms)const;

	std::vector<BoneAnimation> BoneAnimations;
};

struct BoneKeyFrameMatrixs
{
	std::vector<DirectX::XMFLOAT4X4> keyFrameMatrixs;
	float startTime{};
	float endTime{ 1.0f };
	float currentFrameTime{};

	XMFLOAT4X4 GetMatrixByTime(float frameTime)
	{
		currentFrameTime += frameTime;

		if (currentFrameTime >= endTime)
		{
			currentFrameTime = 0;
		}

		return keyFrameMatrixs[(int)((currentFrameTime / (endTime - startTime)) * keyFrameMatrixs.size())];
	}

	XMFLOAT4X4 GetMatrixByTotalTime(float totalTime) const
	{
		totalTime = std::fmod(totalTime, endTime);

		return keyFrameMatrixs[(int)((totalTime / (endTime - startTime)) * keyFrameMatrixs.size())];

	}

	void GetMatrixByTotalTime(float totalTime, DirectX::XMFLOAT4X4& out) const
	{
		totalTime = std::fmod(totalTime, endTime);

		out = keyFrameMatrixs[(int)((totalTime / (endTime - startTime)) * keyFrameMatrixs.size())];
	}
};

class SkinnedData
{
public:

	UINT BoneCount()const;

	float GetClipStartTime(const std::string& clipName)const;
	float GetClipEndTime(const std::string& clipName)const;

	void Set(
		std::vector<int>& boneHierarchy,
		std::vector<DirectX::XMFLOAT4X4>& boneOffsets,
		std::unordered_map<std::string, AnimationClip>& animations);

	// In a real project, you'd want to cache the result if there was a chance
	// that you were calling this several times with the same clipName at 
	// the same timePos.
	void GetFinalTransforms(const std::string& clipName, float timePos,
		std::vector<DirectX::XMFLOAT4X4>& finalTransforms)const;

private:
	// Gives parentIndex of ith bone.
	std::vector<int> mBoneHierarchy;

	std::vector<DirectX::XMFLOAT4X4> mBoneOffsets;

	std::unordered_map<std::string, AnimationClip> mAnimations;
};





void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
void DestroySdkObjects(FbxManager* pManager, bool pExitStatus);
void CreateAndFillIOSettings(FbxManager* pManager);

bool SaveScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename, int pFileFormat=-1, bool pEmbedMedia=false);
bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);

#endif // #ifndef _COMMON_H


