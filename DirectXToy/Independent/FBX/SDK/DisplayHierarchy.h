#pragma once
/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/



#include "DisplayCommon.h"


struct PrimitiveGroup
{
	int start{};
	int end{};
	int materialIndex{};
	int textureIndex{};
};


struct DisplayHierachy
{


	FbxAMatrix GetPoseMatrix(FbxPose *pfbxPose, int nIndex);
	FbxAMatrix GetGlobalPosition(FbxNode *pfbxNode, const FbxTime& pTime, FbxPose *pfbxPose, FbxAMatrix *pfbxmtxParent);
	void MatrixScale(FbxAMatrix& fbxmtxSrcMatrix, double pValue);
	void MatrixAddToDiagonal(FbxAMatrix& fbxmtxSrcMatrix, double pValue);
	void MatrixAdd(FbxAMatrix& fbxmtxDstMatrix, FbxAMatrix& fbxmtxSrcMatrix);
	FbxAMatrix GetGeometricTransform(FbxNode *pfbxNode);
	void DisplayPolygonVertexColors(FbxMesh *pfbxMesh, int nPolygons);
	void DisplayPolygonVertexUVs(FbxMesh* pfbxMesh, int nPolygons, std::vector<SkinnedVertex>& vertexBuffer, std::vector<unsigned int>& indexBuffer);
	void DisplayPolygonVertexUVs2(FbxMesh* pfbxMesh, int nPolygons, std::vector<SkinnedVertex>& vertexBuffer, std::vector<unsigned int>& indexBuffer);
	void DisplayPolygonVertexNormals(FbxMesh* pfbxMesh, int nPolygons, std::vector<SkinnedVertex>& vertexBuffer, std::vector<unsigned int>& indexBuffer);
	void DisplayPolygonVertexTangents(FbxMesh* pfbxMesh, int nPolygons, std::vector<SkinnedVertex>& vertexBuffer, std::vector<unsigned int>& indexBuffer);
	void DisplayPolygonVertexBinormals(FbxMesh *pfbxMesh, int nPolygons);
	void DisplayControlsPoints(FbxMesh *pfbxMesh, int nControlPoints, std::vector<SkinnedVertex>& vertexBuffer,bool isStaticMesh);
	void DisplayPolygonVertexIndices(FbxMesh *pfbxMesh, int nPolygons, std::vector<unsigned int>& indexBuffer);
	void DisplayPolygons(FbxMesh* pfbxMesh, int nPolygons, std::vector<SkinnedVertex>& vertexBuffer, std::vector<unsigned int>& indexBuffer);
	void DisplayMesh(FbxMesh *pfbxMesh, std::vector<SkinnedVertex>& vertexBuffer, std::vector<unsigned int>& indexBuffer , bool doRootTransform);
	void DisplaySkinDeformations(FbxMesh *pfbxMesh, std::vector<SkinnedVertex>& vertices, std::vector<XMFLOAT4X4>& boneOffsets);
	void GetKeyFrameMatrixGlobalNonSkinned(FbxNode* startNode, std::vector<std::vector< std::vector<FbxAMatrix>>>& amazing);
	void GetKeyFrameMatrixGlobalNonSkinned(FbxNode* startNode, std::vector< std::vector<FbxAMatrix>>& amazing);


	void PrintMatrix(FbxAMatrix& matrix);




	void DisplayHierarchy(FbxScene *pfbxScene, std::vector<SkinnedVertex>& vertexBuffer,
		std::vector<unsigned int>& indexBuffer, std::vector<int>& hierachy,  std::vector<XMFLOAT4X4>& boneOffsets,std::vector<std::string>& boneNameList);
	void DisplayHierarchy(FbxNode *pfbxNode,int *pnFrame, int parent, std::vector<SkinnedVertex>& vertexBuffer,
		std::vector<unsigned int>& indexBuffer, std::vector<int>& hierachy, std::vector<XMFLOAT4X4>& boneOffsets, std::vector<std::string>& boneNameList);


	void GetBoneNames(FbxNode* startNode, std::vector<std::vector<std::string>>& output);
	void GetMeshes(FbxNode* pfbxNode, std::vector<std::vector<SkinnedVertex>>& vertexBuffers, std::vector<std::vector<unsigned int>>& indexBuffers, bool isOnlySkinned,bool doRootTransform);

	void GetOffsets(FbxNode* pfbxNode, std::vector<std::vector<SkinnedVertex>>& vertexBuffers,
		 std::vector<std::vector<XMFLOAT4X4>>& boneOffsets, bool first);

	void GetHierarchies(FbxNode *pfbxNode, const std::vector<std::vector<std::string>>& boneNameLists, std::vector<std::vector<int>>& hierarchies);
	void GetHierarchies(FbxNode *pfbxNode, int *pnFrame, int parent, const std::vector<std::string>& boneNameLists, std::vector<int>& hierarchies);

	void GetMaterialGroups(FbxNode* startNode, std::vector<std::vector<PrimitiveGroup>>& primitiveGroups, const string fileName);

	int GetAnimationCurves(FbxAnimLayer *pfbxAnimationLayer, FbxNode *pfbxNode);
	int GetAnimationLayerCurveNodes(FbxAnimLayer *pfbxAnimationLayer, FbxNode *pfbxNode);
	void DisplayCurveKeys(FbxAnimCurve *pfbxAnimationCurve, bool bRotationAngle);
	void DisplayChannels(FbxAnimLayer *pfbxAnimationLayer, FbxNode *pfbxNode, int *pnCurveNode);
	void DisplayAnimation(FbxAnimLayer *pfbxAnimationLayer, FbxNode *pfbxNode, int *pnCurveNode);
	void DisplayAnimation(FbxAnimStack *pfbxAnimStack, FbxNode *pfbxNode);
	void DisplayAnimation(FbxScene *pfbxScene, std::vector<XMFLOAT4X4> boneOffsets, std::vector<int> boneIndexToParentIndex, std::unordered_map<std::string, AnimationClip> animations);



	void GetKeyFrameMatrixLocal(FbxNode* startNode, const std::vector<std::vector<std::string>>& boneNameList, std::vector<std::vector< std::vector<FbxAMatrix>>>& amazing);
	void GetKeyFrameMatrixLocal(FbxNode* startNode, const std::vector<std::string>& boneNameList, std::vector< std::vector<FbxAMatrix>>& amazing);

	void GetKeyFrameMatrixGlobal(FbxNode* startNode, const std::vector<std::vector<std::string>>& boneNameList, std::vector<std::vector< std::vector<FbxAMatrix>>>& amazing);
	void GetKeyFrameMatrixGlobal(FbxNode* startNode, const std::vector<std::string>& boneNameList, std::vector< std::vector<FbxAMatrix>>& amazing);

	int GetSkinnedMeshCount(FbxNode* startNode, bool firstGeneration);
};

