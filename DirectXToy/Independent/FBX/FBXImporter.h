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

using namespace DirectX;


//�޽� -> �� -> ���� Ű������ ���ɺ�ȯ or ������ or ���̸� or ���� or �ε��� or ��������
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