#pragma once
#include "SDK/DisplayHierarchy.h"
#include "SDK/DisplayCommon.h"
#include "SDK/Common.h"
#include "SDK/DisplayAnimation.h"
#include <DirectXMath.h>
#include <string>
#include <vector>

/*
FBX SDK dependency
*/
#include <fbxarray.h>

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

//∏ﬁΩ¨ -> ª¿ -> ª¿¿« ≈∞«¡∑π¿” æ∆«…∫Ø»Ø or ø¿«¡º¬ or ª¿¿Ã∏ß or ¡§¡° or ¿Œµ¶Ω∫ or ∞Ë√˛±∏¡∂
struct Capsule
{
	std::vector<std::vector<BoneKeyFrameMatrixs>> memberBoneKeyFrames;
	std::vector<std::vector<int>> memberHierarchies;
	std::vector<std::vector<XMFLOAT4X4>> memberBoneOffsets;
	std::vector<std::vector<std::string>> memberBoneNameList;
	std::vector<std::vector<SkinnedVertex>> vertices;
	std::vector<std::vector<unsigned int>> indices;
	std::vector<std::string> animationClipNames;
	std::vector<std::vector< PrimitiveGroup>> primitiveGroups;
};


class FBXImporter
{
public:
	enum class OffsetOption
	{
		Local,
		Global
	};
	int memberNumberAnimationStack{};
	std::vector<float> memberStartTimes;
	std::vector<float> memberEndTimes;
	FbxArray<FbxString*> memberAnimationClipNameArray{};

	~FBXImporter();
	void BuildFBXRenderItem(std::string filePath, OffsetOption offsetOption, Capsule* capsule);
	bool SetCurrentAnimationStack(FbxScene* pfbxScene, int index);
};