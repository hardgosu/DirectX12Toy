/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include <fbxsdk.h>
#include "DisplayCommon.h"
#include "DisplayHierarchy.h"
#include <iostream>

using namespace DirectX;


#if defined (FBXSDK_ENV_MAC)
// disable the 밼ormat not a string literal and no format arguments?warning since
// the PrintToFile calls made here are all valid calls and there is no secuity risk
#pragma GCC diagnostic ignored "-Wformat-security"
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
FbxAMatrix DisplayHierachy::GetPoseMatrix(FbxPose* pfbxPose, int nIndex)
{
	FbxAMatrix fbxPoseMatrix;
	FbxMatrix fbxMatrix = pfbxPose->GetMatrix(nIndex);
	memcpy((double*)fbxPoseMatrix, (double*)fbxMatrix, sizeof(fbxMatrix.mData));

	return(fbxPoseMatrix);
}

FbxAMatrix DisplayHierachy::GetGlobalPosition(FbxNode* pfbxNode, const FbxTime& pTime, FbxPose* pfbxPose, FbxAMatrix* pfbxmtxParent)
{
	FbxAMatrix fbxmtxGlobal;
	bool bPositionFound = false;

	if (pfbxPose)
	{
		int nNodeIndex = pfbxPose->Find(pfbxNode);
		if (nNodeIndex >= 0)
		{
			if (pfbxPose->IsBindPose() || !pfbxPose->IsLocalMatrix(nNodeIndex))
			{
				fbxmtxGlobal = GetPoseMatrix(pfbxPose, nNodeIndex);
			}
			else
			{
				FbxAMatrix fbxmtxParent;
				if (pfbxmtxParent)
					fbxmtxParent = *pfbxmtxParent;
				else
				{
					if (pfbxNode->GetParent()) fbxmtxParent = GetGlobalPosition(pfbxNode->GetParent(), pTime, pfbxPose, NULL);
				}
				FbxAMatrix fbxmtxLocal = GetPoseMatrix(pfbxPose, nNodeIndex);
				fbxmtxGlobal = fbxmtxParent * fbxmtxLocal; //Column Major Matrix, Right Handed Coordinates
			}
			bPositionFound = true;
		}
	}
	if (!bPositionFound) fbxmtxGlobal = pfbxNode->EvaluateGlobalTransform(pTime);

	return(fbxmtxGlobal);
}

void DisplayHierachy::MatrixScale(FbxAMatrix& fbxmtxSrcMatrix, double pValue)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++) fbxmtxSrcMatrix[i][j] *= pValue;
	}
}

void DisplayHierachy::MatrixAddToDiagonal(FbxAMatrix& fbxmtxSrcMatrix, double pValue)
{
	fbxmtxSrcMatrix[0][0] += pValue;
	fbxmtxSrcMatrix[1][1] += pValue;
	fbxmtxSrcMatrix[2][2] += pValue;
	fbxmtxSrcMatrix[3][3] += pValue;
}

void DisplayHierachy::MatrixAdd(FbxAMatrix& fbxmtxDstMatrix, FbxAMatrix& fbxmtxSrcMatrix)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++) fbxmtxDstMatrix[i][j] += fbxmtxSrcMatrix[i][j];
	}
}

FbxAMatrix DisplayHierachy::GetGeometricTransform(FbxNode* pfbxNode)
{
	const FbxVector4 T = pfbxNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 R = pfbxNode->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 S = pfbxNode->GetGeometricScaling(FbxNode::eSourcePivot);

	return(FbxAMatrix(T, R, S));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void DisplayHierachy::DisplayPolygonVertexColors(FbxMesh* pfbxMesh, int nPolygons)
{
	int nColorsPerVertex = pfbxMesh->GetElementVertexColorCount();
	if (nColorsPerVertex > 0)
	{
		int nColors = 0;
		for (int i = 0; i < nPolygons; i++) nColors += pfbxMesh->GetPolygonSize(i) * nColorsPerVertex;

		for (int k = 0; k < nColorsPerVertex; k++)
		{

			FbxGeometryElementVertexColor* pfbxElementVertexColor = pfbxMesh->GetElementVertexColor(k);
			for (int i = 0, nVertexID = 0; i < nPolygons; i++)
			{
				int nPolygonSize = pfbxMesh->GetPolygonSize(i); //Triangle: 3, Triangulate()
				for (int j = 0; j < nPolygonSize; j++, nVertexID++)
				{
					int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
					switch (pfbxElementVertexColor->GetMappingMode())
					{
					case FbxGeometryElement::eByControlPoint:
						switch (pfbxElementVertexColor->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
							break;
						case FbxGeometryElement::eIndexToDirect:
							break;
						default:
							break;
						}
						break;
					case FbxGeometryElement::eByPolygonVertex:
						switch (pfbxElementVertexColor->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
							break;
						case FbxGeometryElement::eIndexToDirect:
							break;
						default:
							break;
						}
						break;
					case FbxGeometryElement::eByPolygon:
					case FbxGeometryElement::eAllSame:
					case FbxGeometryElement::eNone:
						break;
					default:
						break;
					}
				}
			}
		}
	}
}
void DisplayHierachy::DisplayPolygonVertexUVs2(FbxMesh* pfbxMesh, int nPolygons, std::vector<SkinnedVertex>& vertexBuffer, std::vector<unsigned int>& indexBuffer)
{


}
void DisplayHierachy::DisplayPolygonVertexUVs(FbxMesh* pfbxMesh, int nPolygons, std::vector<SkinnedVertex>& vertexBuffer, std::vector<unsigned int>& indexBuffer)
{

	std::vector<int> forAverage(vertexBuffer.size());



	int nUVsPerVertex = pfbxMesh->GetElementUVCount(); //UVs Per Polygon's Vertex
	if (nUVsPerVertex > 0)
	{
		int nUVs = 0;
		for (int i = 0; i < nPolygons; i++) nUVs += pfbxMesh->GetPolygonSize(i) * nUVsPerVertex;



		for (int k = 0; k < nUVsPerVertex; k++)
		{
			int index = 0;
			for (int i = 0; i < nPolygons; i++)
			{
				int nPolygonSize = pfbxMesh->GetPolygonSize(i); //Triangle: 3, Triangulate()
				for (int j = 0; j < nPolygonSize; j++)
				{
					int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
					FbxGeometryElementUV* pfbxElementUV = pfbxMesh->GetElementUV(k);

					pfbxElementUV->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
					pfbxElementUV->SetReferenceMode(FbxGeometryElement::eIndexToDirect);



					switch (pfbxElementUV->GetMappingMode())
					{
					case FbxGeometryElement::eByControlPoint:
						switch (pfbxElementUV->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
							//Display2DVector(pfbxElementUV->GetDirectArray().GetAt(nControlPointIndex));
							vertexBuffer[nControlPointIndex].textureCoordinate_.x = pfbxElementUV->GetDirectArray().GetAt(nControlPointIndex).mData[0];
							vertexBuffer[nControlPointIndex].textureCoordinate_.y = pfbxElementUV->GetDirectArray().GetAt(nControlPointIndex).mData[1];

							break;
						case FbxGeometryElement::eIndexToDirect:
							//Display2DVector(pfbxElementUV->GetDirectArray().GetAt(pfbxElementUV->GetIndexArray().GetAt(nControlPointIndex)));
							vertexBuffer[nControlPointIndex].textureCoordinate_.x = pfbxElementUV->GetDirectArray().GetAt(pfbxElementUV->GetIndexArray().GetAt(nControlPointIndex)).mData[0];
							vertexBuffer[nControlPointIndex].textureCoordinate_.y = pfbxElementUV->GetDirectArray().GetAt(pfbxElementUV->GetIndexArray().GetAt(nControlPointIndex)).mData[1];

							break;
						default:
							break;
						}
						break;
					case FbxGeometryElement::eByPolygonVertex:
					{
						int nTextureUVIndex = pfbxMesh->GetTextureUVIndex(i, j);
						switch (pfbxElementUV->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
						case FbxGeometryElement::eIndexToDirect:
							//Display2DVector(pfbxElementUV->GetDirectArray().GetAt(nTextureUVIndex));
							if (indexBuffer[index] < vertexBuffer.size())
							{
								vertexBuffer[indexBuffer[index]].textureCoordinate_.x += ((float)pfbxElementUV->GetDirectArray().GetAt(nTextureUVIndex).mData[0]);
								vertexBuffer[indexBuffer[index]].textureCoordinate_.y += (1.0f - (float)pfbxElementUV->GetDirectArray().GetAt(nTextureUVIndex).mData[1]);
								forAverage[indexBuffer[index]]++;
								++index;
							}

							break;
						default:
							break;
						}
					}
					break;
					case FbxGeometryElement::eByPolygon:
					case FbxGeometryElement::eAllSame:
					case FbxGeometryElement::eNone:
						break;
					default:
						break;
					}
				}
			}
			//하나면 충분하지 뭘
			break;
		}
	}

	for (int i{}; i < vertexBuffer.size(); ++i)
	{
		if (forAverage[i])
		{
			vertexBuffer[i].textureCoordinate_.x /= (float)forAverage[i];
			vertexBuffer[i].textureCoordinate_.y /= (float)forAverage[i];
		}
	}
}

void DisplayHierachy::DisplayPolygonVertexNormals(FbxMesh* pfbxMesh, int nPolygons, std::vector<SkinnedVertex>& vertexBuffer, std::vector<unsigned int>& indexBuffer)
{
	int nNormalsPerVertex = pfbxMesh->GetElementNormalCount();
	if (nNormalsPerVertex > 0)
	{
		int nNormals = 0;
		for (int i = 0; i < nPolygons; i++) nNormals += pfbxMesh->GetPolygonSize(i) * nNormalsPerVertex;

		for (int k = 0; k < nNormalsPerVertex; k++)
		{
			FbxGeometryElementNormal* pfbxElementNormal = pfbxMesh->GetElementNormal(k);
			for (int i = 0, nVertexID = 0; i < nPolygons; i++)
			{
				int nPolygonSize = pfbxMesh->GetPolygonSize(i);
				for (int j = 0; j < nPolygonSize; j++, nVertexID++)
				{
					int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
					if (pfbxElementNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						switch (pfbxElementNormal->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
							vertexBuffer[indexBuffer[nVertexID]].normal_.x += (float)pfbxElementNormal->GetDirectArray().GetAt(nVertexID)[0];
							vertexBuffer[indexBuffer[nVertexID]].normal_.y += (float)pfbxElementNormal->GetDirectArray().GetAt(nVertexID)[1];
							vertexBuffer[indexBuffer[nVertexID]].normal_.z += (float)pfbxElementNormal->GetDirectArray().GetAt(nVertexID)[2];
							//Display3DVector(pfbxElementNormal->GetDirectArray().GetAt(nVertexID));
							break;
							//#주의
						case FbxGeometryElement::eIndexToDirect:
							//Display3DVector(pfbxElementNormal->GetDirectArray().GetAt(pfbxElementNormal->GetIndexArray().GetAt(nVertexID)));

							vertexBuffer[indexBuffer[nVertexID]].normal_.x += (float)pfbxElementNormal->GetDirectArray().GetAt(pfbxElementNormal->GetIndexArray().GetAt(nVertexID))[0];
							vertexBuffer[indexBuffer[nVertexID]].normal_.y += (float)pfbxElementNormal->GetDirectArray().GetAt(pfbxElementNormal->GetIndexArray().GetAt(nVertexID))[1];
							vertexBuffer[indexBuffer[nVertexID]].normal_.z += (float)pfbxElementNormal->GetDirectArray().GetAt(pfbxElementNormal->GetIndexArray().GetAt(nVertexID))[2];
							break;
						default:
							break;
						}
					}
				}
			}
		}
	}
}

void DisplayHierachy::DisplayPolygonVertexTangents(FbxMesh* pfbxMesh, int nPolygons, std::vector<SkinnedVertex>& vertexBuffer, std::vector<unsigned int>& indexBuffer)
{
	int nTangentsPerVertex = pfbxMesh->GetElementTangentCount();
	if (nTangentsPerVertex > 0)
	{
		int nTangents = 0;
		for (int i = 0; i < nPolygons; i++) nTangents += pfbxMesh->GetPolygonSize(i) * nTangentsPerVertex;

		for (int k = 0; k < nTangentsPerVertex; k++)
		{
			FbxGeometryElementTangent* pfbxElementTangent = pfbxMesh->GetElementTangent(k);
			for (int i = 0, nVertexID = 0; i < nPolygons; i++)
			{
				int nPolygonSize = pfbxMesh->GetPolygonSize(i);
				for (int j = 0; j < nPolygonSize; j++, nVertexID++)
				{
					int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
					if (pfbxElementTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						switch (pfbxElementTangent->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
							vertexBuffer[indexBuffer[nVertexID]].tangent_.x += (float)pfbxElementTangent->GetDirectArray().GetAt(nVertexID)[0];
							vertexBuffer[indexBuffer[nVertexID]].tangent_.y += (float)pfbxElementTangent->GetDirectArray().GetAt(nVertexID)[1];
							vertexBuffer[indexBuffer[nVertexID]].tangent_.z += (float)pfbxElementTangent->GetDirectArray().GetAt(nVertexID)[2];

							break;
						case FbxGeometryElement::eIndexToDirect:
							break;
						default:
							break;
						}
					}
				}
			}
		}
	}
}

void DisplayHierachy::DisplayPolygonVertexBinormals(FbxMesh* pfbxMesh, int nPolygons)
{
	int nBinormalsPerVertex = pfbxMesh->GetElementTangentCount();
	if (nBinormalsPerVertex > 0)
	{
		int nBinormals = 0;
		for (int i = 0; i < nPolygons; i++) nBinormals += pfbxMesh->GetPolygonSize(i) * nBinormalsPerVertex;

		for (int k = 0; k < nBinormalsPerVertex; k++)
		{
			FbxGeometryElementBinormal* pfbxElementBinormal = pfbxMesh->GetElementBinormal(k);
			for (int i = 0, nVertexID = 0; i < nPolygons; i++)
			{
				int nPolygonSize = pfbxMesh->GetPolygonSize(i);
				for (int j = 0; j < nPolygonSize; j++, nVertexID++)
				{
					if (pfbxElementBinormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						switch (pfbxElementBinormal->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
							break;
						case FbxGeometryElement::eIndexToDirect:
							break;
						default:
							break;
						}
					}
				}
			}
		}
	}
}

void DisplayHierachy::DisplayControlsPoints(FbxMesh* pfbxMesh, int nControlPoints, std::vector<SkinnedVertex>& vertexBuffer, bool doRootTransform)
{
	FbxVector4* pfbxvControlPoints = pfbxMesh->GetControlPoints();
	vertexBuffer.resize(pfbxMesh->GetControlPointsCount());

	FbxNode* node = pfbxMesh->GetNode();
	FbxAMatrix transform = node->EvaluateGlobalTransform();

	for (int i = 0; i < nControlPoints; i++)
	{
		if (doRootTransform)
		{
			pfbxvControlPoints[i] = transform.MultT(pfbxvControlPoints[i]);
		}
		vertexBuffer[i].position_ =
		{
			(float)pfbxvControlPoints[i][0],
			(float)pfbxvControlPoints[i][1],
			(float)pfbxvControlPoints[i][2],
		};

	}
}

void DisplayHierachy::DisplayPolygonVertexIndices(FbxMesh* pfbxMesh, int nPolygons, std::vector<unsigned int>& indexBuffer)
{
	int nIndices = 0;
	for (int i = 0; i < nPolygons; i++) nIndices += pfbxMesh->GetPolygonSize(i); //Triangle: 3, Triangulate(), nIndices = nPolygons * 3

	indexBuffer.resize(nIndices);
	int k{};
	for (int i{}; i < nPolygons; i++)
	{
		int nPolygonSize = pfbxMesh->GetPolygonSize(i);
		for (int j = 0; j < nPolygonSize; j++)
		{
			indexBuffer[k++] = pfbxMesh->GetPolygonVertex(i, j);
		}
	}
}

void DisplayHierachy::DisplayPolygons(FbxMesh* pfbxMesh, int nPolygons, std::vector<SkinnedVertex>& vertexBuffer, std::vector<unsigned int>& indexBuffer)
{

	DisplayPolygonVertexIndices(pfbxMesh, nPolygons, indexBuffer);

	//DisplayPolygonVertexColors(pfbxMesh, nPolygons, nTabIndents + 1);
	DisplayPolygonVertexUVs(pfbxMesh, nPolygons, vertexBuffer, indexBuffer);
	DisplayPolygonVertexNormals(pfbxMesh, nPolygons, vertexBuffer, indexBuffer);
	//DisplayPolygonVertexTangents(pfbxMesh, nPolygons, vertexBuffer, indexBuffer);
	//DisplayPolygonVertexBinormals(pfbxMesh, nPolygons, nTabIndents + 1);

}

void DisplayHierachy::DisplayMesh(FbxMesh* pfbxMesh, std::vector<SkinnedVertex>& vertexBuffer, std::vector<unsigned int>& indexBuffer ,bool doRootTransform)
{
	int nControlPoints = pfbxMesh->GetControlPointsCount();
	if (nControlPoints > 0) DisplayControlsPoints(pfbxMesh, nControlPoints, vertexBuffer, doRootTransform);

	int nPolygons = pfbxMesh->GetPolygonCount();
	if (nPolygons > 0) DisplayPolygons(pfbxMesh, nPolygons, vertexBuffer, indexBuffer);



	//DisplayMaterial(pfbxMesh);
	//DisplayTexture(pfbxMesh);
}






void DisplayHierachy::DisplaySkinDeformations(FbxMesh* pfbxMesh, std::vector<SkinnedVertex>& vertices, std::vector<XMFLOAT4X4>& boneOffsets)
{
	int nControlPoints = pfbxMesh->GetControlPointsCount();

	FbxSkin* pfbxSkinDeformer = (FbxSkin*)pfbxMesh->GetDeformer(0, FbxDeformer::eSkin);
	int nClusters = pfbxSkinDeformer->GetClusterCount();


	for (int j = 0; j < nClusters; j++)
	{
		FbxCluster* pfbxCluster = pfbxSkinDeformer->GetCluster(j);
		FbxNode* pfbxClusterLink = pfbxCluster->GetLink();
	}


	FbxAMatrix fbxmtxGeometryOffset = GetGeometricTransform(pfbxMesh->GetNode());
	FbxAMatrix fbxmtxVertextToLinkNode;
	for (int j = 0; j < nClusters; j++)
	{
		FbxCluster* pfbxCluster = pfbxSkinDeformer->GetCluster(j);

		FbxAMatrix fbxmtxClusterTransform; //Cluster Transform
		pfbxCluster->GetTransformMatrix(fbxmtxClusterTransform);
		FbxAMatrix fbxmtxClusterLinkTransform; //Cluster Link Transform
		pfbxCluster->GetTransformLinkMatrix(fbxmtxClusterLinkTransform);

		fbxmtxVertextToLinkNode = fbxmtxClusterLinkTransform.Inverse() * fbxmtxClusterTransform * fbxmtxGeometryOffset;
		//fbxmtxVertextToLinkNode = fbxmtxGeometryOffset.Transpose() * fbxmtxClusterTransform.Transpose() * fbxmtxClusterLinkTransform.Inverse().Transpose();

		const auto& a = fbxmtxVertextToLinkNode;
		//a.SetIdentity();

		XMFLOAT4X4 offset
		{
			(float)a.mData[0][0],(float)a.mData[0][1],(float)a.mData[0][2],(float)a.mData[0][3],
			(float)a.mData[1][0],(float)a.mData[1][1],(float)a.mData[1][2],(float)a.mData[1][3],
			(float)a.mData[2][0],(float)a.mData[2][1],(float)a.mData[2][2],(float)a.mData[2][3],
			(float)a.mData[3][0],(float)a.mData[3][1],(float)a.mData[3][2],(float)a.mData[3][3]

		};

		boneOffsets.emplace_back(offset);
	}


	int* pnBonesPerVertex = new int[nControlPoints];
	std::memset(pnBonesPerVertex, 0, nControlPoints * sizeof(int));
	for (int j = 0; j < nClusters; j++)
	{
		FbxCluster* pfbxCluster = pfbxSkinDeformer->GetCluster(j);
		int nControlPointIndices = pfbxCluster->GetControlPointIndicesCount();
		int* pnControlPointIndices = pfbxCluster->GetControlPointIndices();
		for (int k = 0; k < nControlPointIndices; k++) pnBonesPerVertex[pnControlPointIndices[k]]++;
	}
	int nMaxBonesPerVertex = 0;
	for (int i = 0; i < nControlPoints; i++) if (pnBonesPerVertex[i] > nMaxBonesPerVertex) nMaxBonesPerVertex = pnBonesPerVertex[i];

	int** ppnBoneIDs = new int* [nControlPoints];
	float** ppnBoneWeights = new float* [nControlPoints];
	for (int i = 0; i < nControlPoints; i++)
	{
		ppnBoneIDs[i] = new int[pnBonesPerVertex[i]];
		ppnBoneWeights[i] = new float[pnBonesPerVertex[i]];
		::memset(ppnBoneIDs[i], 0, pnBonesPerVertex[i] * sizeof(int));
		::memset(ppnBoneWeights[i], 0, pnBonesPerVertex[i] * sizeof(float));
	}

	int* pnBones = new int[nControlPoints];
	::memset(pnBones, 0, nControlPoints * sizeof(int));

	for (int j = 0; j < nClusters; j++)
	{
		FbxCluster* pfbxCluster = pfbxSkinDeformer->GetCluster(j);

		int* pnControlPointIndices = pfbxCluster->GetControlPointIndices();
		double* pfControlPointWeights = pfbxCluster->GetControlPointWeights();
		int nControlPointIndices = pfbxCluster->GetControlPointIndicesCount();

		for (int k = 0; k < nControlPointIndices; k++)
		{
			int nVertex = pnControlPointIndices[k];
			ppnBoneIDs[nVertex][pnBones[nVertex]] = j;
			ppnBoneWeights[nVertex][pnBones[nVertex]++] = (float)pfControlPointWeights[k];
		}
	}

	float* pnSumOfBoneWeights = new float[nControlPoints];
	::memset(pnSumOfBoneWeights, 0, nControlPoints * sizeof(float));

	for (int i = 0; i < nControlPoints; i++)
	{
		for (int j = 0; j < pnBonesPerVertex[i]; j++) pnSumOfBoneWeights[i] += ppnBoneWeights[i][j];
		for (int j = 0; j < pnBonesPerVertex[i]; j++) ppnBoneWeights[i][j] /= pnSumOfBoneWeights[i];
	}

	if (pnSumOfBoneWeights) delete[] pnSumOfBoneWeights;

	for (int i = 0; i < nControlPoints; i++)
	{
		for (int j = 0; j < pnBonesPerVertex[i] - 1; j++)
		{
			for (int k = j + 1; k < pnBonesPerVertex[i]; k++)
			{
				if (ppnBoneWeights[i][j] < ppnBoneWeights[i][k])
				{
					float fTemp = ppnBoneWeights[i][j];
					ppnBoneWeights[i][j] = ppnBoneWeights[i][k];
					ppnBoneWeights[i][k] = fTemp;
					int nTemp = ppnBoneIDs[i][j];
					ppnBoneIDs[i][j] = ppnBoneIDs[i][k];
					ppnBoneIDs[i][k] = nTemp;
				}
			}
		}
	}

	int(*pnSkinningIndices)[4] = new int[nControlPoints][4];
	float(*pfSkinningWeights)[4] = new float[nControlPoints][4];

	for (int i = 0; i < nControlPoints; i++)
	{
		::memset(pnSkinningIndices[i], 0, 4 * sizeof(int));
		::memset(pfSkinningWeights[i], 0, 4 * sizeof(float));

		for (int j = 0; j < pnBonesPerVertex[i]; j++)
		{
			if (j < 4)
			{
				pnSkinningIndices[i][j] = ppnBoneIDs[i][j];
				pfSkinningWeights[i][j] = ppnBoneWeights[i][j];
			}
		}
	}

	for (int i = 0; i < nControlPoints; i++)
	{
		vertices[i].boneIndices_[0] = pnSkinningIndices[i][0];
		vertices[i].boneIndices_[1] = pnSkinningIndices[i][1];
		vertices[i].boneIndices_[2] = pnSkinningIndices[i][2];
		vertices[i].boneIndices_[3] = pnSkinningIndices[i][3];

		vertices[i].boneWeights_.x = pfSkinningWeights[i][0];
		vertices[i].boneWeights_.y = pfSkinningWeights[i][1];
		vertices[i].boneWeights_.z = pfSkinningWeights[i][2];
	}


	for (int i = 0; i < nControlPoints; i++)
	{
		if (ppnBoneIDs[i]) delete[] ppnBoneIDs[i];
		if (ppnBoneWeights[i]) delete[] ppnBoneWeights[i];
	}
	if (ppnBoneIDs) delete[] ppnBoneIDs;
	if (ppnBoneWeights) delete[] ppnBoneWeights;

	if (pnBones) delete[] pnBones;
	if (pnBonesPerVertex) delete[] pnBonesPerVertex;
	if (pnSkinningIndices) delete[] pnSkinningIndices;
	if (pfSkinningWeights) delete[] pfSkinningWeights;
}

void DisplayHierachy::PrintMatrix(FbxAMatrix& matrix)
{
	for (int i{}; i < 4; ++i)
	{
		for (int j{}; j < 4; ++j)
		{
			std::cout << matrix.mData[j][i] << " ";

		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}





void DisplayHierachy::GetBoneNames(FbxNode* startNode, std::vector<std::vector<std::string>>& output)
{

	FbxNodeAttribute* localNodeAttribute = startNode->GetNodeAttribute();
	if (localNodeAttribute)
	{
		FbxNodeAttribute::EType nAttributeType = localNodeAttribute->GetAttributeType();
		if (nAttributeType == FbxNodeAttribute::eMesh)
		{
			FbxMesh* pfbxMesh = (FbxMesh*)startNode->GetNodeAttribute();
			int nSkinDeformers = pfbxMesh->GetDeformerCount(FbxDeformer::eSkin);
			if (nSkinDeformers > 0)
			{
				output.emplace_back();
				FbxSkin* pfbxSkinDeformer = (FbxSkin*)pfbxMesh->GetDeformer(0, FbxDeformer::eSkin);
				int nClusters = pfbxSkinDeformer->GetClusterCount();
				for (int j = 0; j < nClusters; j++)
				{
					FbxCluster* pfbxCluster = pfbxSkinDeformer->GetCluster(j);
					FbxNode* pfbxClusterLink = pfbxCluster->GetLink();
					//DisplayString("", ReplaceBlank((char *)pfbxClusterLink->GetName(), '_'), " ");

					output.back().push_back((char*)pfbxClusterLink->GetName());
				}
			}

		}
	}

	int localNumberChild = startNode->GetChildCount();

	for (int i{}; i < localNumberChild; ++i)
	{
		GetBoneNames(startNode->GetChild(i), output);
	}

}


int DisplayHierachy::GetSkinnedMeshCount(FbxNode* startNode, bool firstGeneration)
{
	static int skinnedMeshCount{};
	if (firstGeneration)
	{
		skinnedMeshCount = 0;
	}

	FbxNodeAttribute* pfbxNodeAttribute = startNode->GetNodeAttribute();
	if (pfbxNodeAttribute)
	{
		FbxNodeAttribute::EType nAttributeType = pfbxNodeAttribute->GetAttributeType();
		if (nAttributeType == FbxNodeAttribute::eMesh)
		{
			FbxMesh* pfbxMesh = (FbxMesh*)startNode->GetNodeAttribute();

			int nSkinDeformers = pfbxMesh->GetDeformerCount(FbxDeformer::eSkin);
			if (nSkinDeformers > 0)
			{
				++skinnedMeshCount;
			}
		}
	}

	int nChilds = startNode->GetChildCount();

	for (int i{}; i < nChilds; ++i)
	{
		GetSkinnedMeshCount(startNode->GetChild(i), false);
	}

	return skinnedMeshCount;
}
//#주의
void DisplayHierachy::GetMeshes(FbxNode* pfbxNode, std::vector<std::vector<SkinnedVertex>>& vertexBuffers, std::vector<std::vector<unsigned int>>& indexBuffers, bool isOnlySkinned,bool doRootTransform)
{
	//FbxAMatrix fbxmtxLocalTransform = pfbxNode->EvaluateLocalTransform(FBXSDK_TIME_INFINITE);
	FbxNodeAttribute* pfbxNodeAttribute = pfbxNode->GetNodeAttribute();
	if (pfbxNodeAttribute)
	{
		FbxNodeAttribute::EType nAttributeType = pfbxNodeAttribute->GetAttributeType();
		if (nAttributeType == FbxNodeAttribute::eMesh)
		{
			FbxMesh* pfbxMesh = (FbxMesh*)pfbxNode->GetNodeAttribute();
			if (isOnlySkinned)
			{
				int nSkinDeformers = pfbxMesh->GetDeformerCount(FbxDeformer::eSkin);
				if (nSkinDeformers > 0)
				{
					vertexBuffers.emplace_back();

					indexBuffers.emplace_back();

					DisplayMesh(pfbxMesh, vertexBuffers.back(), indexBuffers.back(), doRootTransform);
				}

			}
			else
			{
				vertexBuffers.emplace_back();

				indexBuffers.emplace_back();

				DisplayMesh(pfbxMesh, vertexBuffers.back(), indexBuffers.back(), doRootTransform);
			}
		}
	}
	int nChilds = pfbxNode->GetChildCount();

	for (int i{}; i < nChilds; ++i)
	{
		GetMeshes(pfbxNode->GetChild(i), vertexBuffers, indexBuffers, isOnlySkinned, doRootTransform);
	}
}

void DisplayHierachy::GetHierarchies(FbxNode* pfbxNode, const std::vector<std::vector<std::string>>& boneNameLists, std::vector<std::vector<int>>& hierarchies)
{
	int nChilds = pfbxNode->GetChildCount();
	int k{};

	for (int j{}; j < boneNameLists.size(); ++j)
	{
		int nFrame = 0;
		hierarchies.emplace_back();
		for (int i{}; i < nChilds; ++i)
		{
			GetHierarchies(pfbxNode->GetChild(i), &nFrame, -1, boneNameLists[j], hierarchies.back());
		}
	}
}


void DisplayHierachy::GetHierarchies(FbxNode* pfbxNode, int* pnFrame, int parent, const std::vector<std::string>& boneNameList, std::vector<int>& hierarchy)
{
	int nChilds = pfbxNode->GetChildCount();
	//std::cout << pfbxNode->GetName() << std::endl;
	//if (boneNameList.count(pfbxNode->GetName()) > 0)
	if (std::find(boneNameList.begin(), boneNameList.end(), pfbxNode->GetName()) != boneNameList.end())
	{
		int current = *pnFrame;
		(*pnFrame)++;

		hierarchy.push_back(parent);

		for (int i = 0; i < nChilds; i++)
		{

			GetHierarchies(pfbxNode->GetChild(i), pnFrame, current, boneNameList, hierarchy);
		}
	}
	else
	{
		for (int i = 0; i < nChilds; i++)
		{
			GetHierarchies(pfbxNode->GetChild(i), pnFrame, parent, boneNameList, hierarchy);

		}
	}
}



void DisplayHierachy::DisplayHierarchy(FbxScene* pfbxScene, std::vector<SkinnedVertex>& vertexBuffer,
	std::vector<unsigned int>& indexBuffer, std::vector<int>& hierachy, std::vector<XMFLOAT4X4>& boneOffsets, std::vector<std::string>& boneNameList)
{
	FbxNode* pfbxRootNode = pfbxScene->GetRootNode();

	//std::cout << pfbxRootNode->GetChildCount() << std::endl;

	int nFrame = 0;
	for (int i = 0; i < pfbxRootNode->GetChildCount(); i++)
	{
		DisplayHierarchy(pfbxRootNode->GetChild(i), &nFrame, -1, vertexBuffer, indexBuffer, hierachy, boneOffsets, boneNameList);

		nFrame = 0;
	}
}

void DisplayHierachy::GetOffsets(FbxNode* pfbxNode, std::vector<std::vector<SkinnedVertex>>& vertexBuffers,
	std::vector<std::vector<XMFLOAT4X4>>& boneOffsets, bool first)
{
	static int a{};

	if (first)
		a = 0;

	FbxAMatrix fbxmtxLocalTransform = pfbxNode->EvaluateLocalTransform(FBXSDK_TIME_INFINITE);
	FbxNodeAttribute* pfbxNodeAttribute = pfbxNode->GetNodeAttribute();
	if (pfbxNodeAttribute)
	{
		FbxNodeAttribute::EType nAttributeType = pfbxNodeAttribute->GetAttributeType();
		if (nAttributeType == FbxNodeAttribute::eMesh)
		{
			FbxMesh* pfbxMesh = (FbxMesh*)pfbxNode->GetNodeAttribute();
			int nSkinDeformers = pfbxMesh->GetDeformerCount(FbxDeformer::eSkin);
			if (nSkinDeformers > 0)
			{
				boneOffsets.emplace_back();
				DisplaySkinDeformations(pfbxMesh, vertexBuffers[a], boneOffsets.back());

				++a;
			}
		}
	}

	int nChilds = pfbxNode->GetChildCount();

	for (int i{}; i < nChilds; ++i)
	{
		GetOffsets(pfbxNode->GetChild(i), vertexBuffers, boneOffsets, false);
	}
}



//지금 이 노드가 사전에 있는가를 따져야한다.
void DisplayHierachy::DisplayHierarchy(FbxNode* pfbxNode, int* pnFrame, int parent, std::vector<SkinnedVertex>& vertexBuffer,
	std::vector<unsigned int>& indexBuffer, std::vector<int>& hierachy, std::vector<XMFLOAT4X4>& boneOffsets, std::vector<std::string>& boneNameList)
{
	int nChilds = pfbxNode->GetChildCount();
	//std::cout << pfbxNode->GetName() << std::endl;
	//if (boneNameList.count(pfbxNode->GetName()) > 0)
	if (std::find(boneNameList.begin(), boneNameList.end(), pfbxNode->GetName()) != boneNameList.end())
	{
		int current = *pnFrame;
		(*pnFrame)++;

		hierachy.push_back(parent);

		FbxAMatrix fbxmtxLocalTransform = pfbxNode->EvaluateLocalTransform(FBXSDK_TIME_INFINITE);
		FbxNodeAttribute* pfbxNodeAttribute = pfbxNode->GetNodeAttribute();
		if (pfbxNodeAttribute)
		{
			FbxNodeAttribute::EType nAttributeType = pfbxNodeAttribute->GetAttributeType();
			if (nAttributeType == FbxNodeAttribute::eMesh)
			{
				FbxMesh* pfbxMesh = (FbxMesh*)pfbxNode->GetNodeAttribute();
				int nSkinDeformers = pfbxMesh->GetDeformerCount(FbxDeformer::eSkin);
				if (nSkinDeformers > 0)
				{

					DisplaySkinDeformations(pfbxMesh, vertexBuffer, boneOffsets);

				}

				DisplayMesh(pfbxMesh, vertexBuffer, indexBuffer, false);

				//중복없는 리스트
				FbxSkin* pfbxSkinDeformer = (FbxSkin*)pfbxMesh->GetDeformer(0, FbxDeformer::eSkin);
				int nClusters = pfbxSkinDeformer->GetClusterCount();
				for (int j = 0; j < nClusters; j++)
				{
					FbxCluster* pfbxCluster = pfbxSkinDeformer->GetCluster(j);
					FbxNode* pfbxClusterLink = pfbxCluster->GetLink();

					boneNameList.push_back((char*)pfbxClusterLink->GetName());
				}
			}
		}

		for (int i = 0; i < nChilds; i++)
		{
			DisplayHierarchy(pfbxNode->GetChild(i), pnFrame, current, vertexBuffer, indexBuffer, hierachy, boneOffsets, boneNameList);
		}
	}
	else
	{
		for (int i = 0; i < nChilds; i++)
		{
			DisplayHierarchy(pfbxNode->GetChild(i), pnFrame, parent, vertexBuffer, indexBuffer, hierachy, boneOffsets, boneNameList);
		}
	}
}







int DisplayHierachy::GetAnimationCurves(FbxAnimLayer* pfbxAnimationLayer, FbxNode* pfbxNode)
{
	int nAnimationCurves = 0;

	FbxAnimCurve* pfbxAnimationCurve = NULL;
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

int DisplayHierachy::GetAnimationLayerCurveNodes(FbxAnimLayer* pfbxAnimationLayer, FbxNode* pfbxNode)
{
	int nAnimationCurveNodes = 0;
	if (GetAnimationCurves(pfbxAnimationLayer, pfbxNode) > 0) nAnimationCurveNodes = 1;

	for (int i = 0; i < pfbxNode->GetChildCount(); i++)
	{
		nAnimationCurveNodes += GetAnimationLayerCurveNodes(pfbxAnimationLayer, pfbxNode->GetChild(i));
	}

	return(nAnimationCurveNodes);
}

void DisplayHierachy::DisplayCurveKeys(FbxAnimCurve* pfbxAnimationCurve, bool bRotationAngle)
{
	int nKeys = pfbxAnimationCurve->KeyGetCount();
	//DisplayInt(pHeader, nKeys, " ", nTabIndents);

	for (int i = 0; i < nKeys; i++)
	{
		FbxTime fbxKeyTime = pfbxAnimationCurve->KeyGetTime(i);
		float fkeyTime = (float)fbxKeyTime.GetSecondDouble();
		//DisplayFloat("", fkeyTime, " ");
	}
	for (int i = 0; i < nKeys; i++)
	{
		float fKeyValue = static_cast<float>(pfbxAnimationCurve->KeyGetValue(i));
		if (bRotationAngle) fKeyValue = (3.1415926f / 180.f) * fKeyValue;
		//DisplayFloat("", fKeyValue, " ");
	}

	//printf_s("\n");
}

void DisplayHierachy::DisplayChannels(FbxAnimLayer* pfbxAnimationLayer, FbxNode* pfbxNode, int* pnCurveNode)
{
	if (GetAnimationCurves(pfbxAnimationLayer, pfbxNode) > 0)
	{
		//DisplayIntString("<AnimationCurve>: ", (*pnCurveNode)++, ReplaceBlank(pfbxNode->GetName(), '_'), "\n", nTabIndents);

		FbxAnimCurve* pfbxAnimationCurve = NULL;

		//FbxAnimCurveNode *pfbxAnimCurveNode = pfbxNode->LclTranslation.GetCurveNode(pfbxAnimationLayer);
		//int nChannels = pfbxAnimCurveNode->GetChannelsCount();
		//int nCurves = pfbxAnimCurveNode->GetCurveCount(0);

		pfbxAnimationCurve = pfbxNode->LclTranslation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_X); //"X"
		//if (pfbxAnimationCurve) DisplayCurveKeys("<TX>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, false);
		if (pfbxAnimationCurve) DisplayCurveKeys(pfbxAnimationCurve, false);

		pfbxAnimationCurve = pfbxNode->LclTranslation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		//if (pfbxAnimationCurve) DisplayCurveKeys("<TY>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, false);
		if (pfbxAnimationCurve) DisplayCurveKeys(pfbxAnimationCurve, false);

		pfbxAnimationCurve = pfbxNode->LclTranslation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		//if (pfbxAnimationCurve) DisplayCurveKeys("<TZ>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, false);
		if (pfbxAnimationCurve) DisplayCurveKeys(pfbxAnimationCurve, false);

		pfbxAnimationCurve = pfbxNode->LclRotation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_X);
		//if (pfbxAnimationCurve) DisplayCurveKeys("<RX>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, true);
		if (pfbxAnimationCurve) DisplayCurveKeys(pfbxAnimationCurve, true);

		pfbxAnimationCurve = pfbxNode->LclRotation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		//if (pfbxAnimationCurve) DisplayCurveKeys("<RY>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, true);
		if (pfbxAnimationCurve) DisplayCurveKeys(pfbxAnimationCurve, true);

		pfbxAnimationCurve = pfbxNode->LclRotation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		//if (pfbxAnimationCurve) DisplayCurveKeys("<RZ>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, true);
		if (pfbxAnimationCurve) DisplayCurveKeys(pfbxAnimationCurve, true);

		pfbxAnimationCurve = pfbxNode->LclScaling.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_X);
		//if (pfbxAnimationCurve) DisplayCurveKeys("<SX>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, false);
		if (pfbxAnimationCurve) DisplayCurveKeys(pfbxAnimationCurve, false);

		pfbxAnimationCurve = pfbxNode->LclScaling.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		//if (pfbxAnimationCurve) DisplayCurveKeys("<SY>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, false);
		if (pfbxAnimationCurve) DisplayCurveKeys(pfbxAnimationCurve, false);

		pfbxAnimationCurve = pfbxNode->LclScaling.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		//if (pfbxAnimationCurve) DisplayCurveKeys("<SZ>: ", pfbxAnimationCurve, "\n", nTabIndents + 1, false);
		if (pfbxAnimationCurve) DisplayCurveKeys(pfbxAnimationCurve, false);

		//DisplayString("</AnimationCurve>", "", "\n", nTabIndents);
	}
}

void DisplayHierachy::DisplayAnimation(FbxAnimLayer* pfbxAnimationLayer, FbxNode* pfbxNode, int* pnCurveNode)
{
	//DisplayChannels(pfbxAnimationLayer, pfbxNode, pnCurveNode);
	DisplayChannels(pfbxAnimationLayer, pfbxNode, pnCurveNode);

	for (int i = 0; i < pfbxNode->GetChildCount(); i++)
	{
		DisplayAnimation(pfbxAnimationLayer, pfbxNode->GetChild(i), pnCurveNode);
	}
}

void DisplayHierachy::DisplayAnimation(FbxAnimStack* pfbxAnimStack, FbxNode* pfbxNode)
{
	int nAnimationLayers = pfbxAnimStack->GetMemberCount<FbxAnimLayer>();
	//DisplayInt("<AnimationLayers>: ", nAnimationLayers, "\n", nTabIndents);

	for (int i = 0; i < nAnimationLayers; i++)
	{
		FbxAnimLayer* pfbxAnimationLayer = pfbxAnimStack->GetMember<FbxAnimLayer>(i);
		int nLayerCurveNodes = GetAnimationLayerCurveNodes(pfbxAnimationLayer, pfbxNode);

		int nCurveNode = 0;
		//DisplayIntFloat("<AnimationLayer>: ", i, nLayerCurveNodes, float(pfbxAnimationLayer->Weight) / 100.0f, "\n", nTabIndents + 1);
		DisplayAnimation(pfbxAnimationLayer, pfbxNode, &nCurveNode);
		//DisplayString("</AnimationLayer>", "", "\n", nTabIndents + 1);
	}

	//DisplayString("</AnimationLayers>", "", "\n", nTabIndents);
}

void DisplayHierachy::DisplayAnimation(FbxScene* pfbxScene, std::vector<XMFLOAT4X4> boneOffsets, std::vector<int> boneIndexToParentIndex, std::unordered_map<std::string, AnimationClip> animations)
{
	int nAnimationStacks = pfbxScene->GetSrcObjectCount<FbxAnimStack>();
	//DisplayInt("<AnimationSets>: ", nAnimationStacks, "\n", 1);

	for (int i = 0; i < nAnimationStacks; i++)
	{
		FbxAnimStack* pfbxAnimStack = pfbxScene->GetSrcObject<FbxAnimStack>(i);
		//DisplayIntString("<AnimationSet>: ", i, ReplaceBlank(pfbxAnimStack->GetName(), '_'), " ", 2);
		FbxTime fbxTimeStart = pfbxAnimStack->LocalStart;
		FbxTime fbxTimeStop = pfbxAnimStack->LocalStop;
		//DisplayFloat("", (float)fbxTimeStart.GetSecondDouble(), (float)fbxTimeStop.GetSecondDouble(), "\n");

		DisplayAnimation(pfbxAnimStack, pfbxScene->GetRootNode());
		//DisplayString("</AnimationSet>", "", "\n", 2);
	}

	//DisplayString("</AnimationSets>", "", "\n", 1);
}

void DisplayHierachy::GetKeyFrameMatrixGlobalNonSkinned(FbxNode* startNode, std::vector<std::vector< std::vector<FbxAMatrix>>>& amazing)
{
	int nChilds = startNode->GetChildCount();
	amazing.emplace_back();

	for (int i{}; i < nChilds; ++i)
	{
		GetKeyFrameMatrixGlobalNonSkinned(startNode->GetChild(i), amazing.back());
	}
}

void DisplayHierachy::GetKeyFrameMatrixGlobalNonSkinned(FbxNode* startNode, std::vector< std::vector<FbxAMatrix>>& amazing)
{

	FbxNodeAttribute* pfbxNodeAttribute = startNode->GetNodeAttribute();
	if (pfbxNodeAttribute && (pfbxNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh))
	{
		FbxMesh* pfbxMesh = startNode->GetMesh();

		FbxScene* scene = pfbxMesh->GetScene();


		FbxAnimStack* pfbxAnimationStack = scene->GetCurrentAnimationStack();

		std::string currentClipName = pfbxAnimationStack->GetName();


		FbxTakeInfo* takeInfo = scene->GetTakeInfo(FbxString(currentClipName.c_str()));

		FbxTime startTime = takeInfo->mLocalTimeSpan.GetStart();
		FbxTime endTime = takeInfo->mLocalTimeSpan.GetStop();

		FbxTime difference = endTime - startTime;

		float test = difference.GetSecondDouble();
		FbxAMatrix keyFrameMatrix;

		int lerpCount = difference.GetSecondDouble() * 60;

		amazing.emplace_back();

		for (int l{}; l < lerpCount; ++l)
		{
			keyFrameMatrix = startNode->EvaluateGlobalTransform(startTime + (difference / lerpCount) * l);
			amazing.back().push_back(keyFrameMatrix);
		}
	}

	int nChilds = startNode->GetChildCount();
	for (int i{}; i < nChilds; ++i)
	{
		GetKeyFrameMatrixGlobalNonSkinned(startNode->GetChild(i), amazing);
	}
}



void DisplayHierachy::GetKeyFrameMatrixGlobal(FbxNode* startNode, const std::vector<std::vector<std::string>>& boneNameList, std::vector<std::vector< std::vector<FbxAMatrix>>>& amazing)
{
	int nChilds = startNode->GetChildCount();

	amazing.emplace_back();

	for (int i{}; i < nChilds; ++i)
	{
		GetKeyFrameMatrixGlobal(startNode->GetChild(i), boneNameList[0], amazing.back());
	}

}
void DisplayHierachy::GetKeyFrameMatrixGlobal(FbxNode* startNode, const std::vector<std::string>& boneNameList, std::vector< std::vector<FbxAMatrix>>& amazing)
{
	FbxNodeAttribute* pfbxNodeAttribute = startNode->GetNodeAttribute();
	if (pfbxNodeAttribute && (pfbxNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh))
	{
		FbxMesh* pfbxMesh = startNode->GetMesh();

		FbxScene* scene = pfbxMesh->GetScene();


		FbxAnimStack* pfbxAnimationStack = scene->GetCurrentAnimationStack();

		std::string currentClipName = pfbxAnimationStack->GetName();


		FbxTakeInfo* takeInfo = scene->GetTakeInfo(FbxString(currentClipName.c_str()));

		FbxTime startTime = takeInfo->mLocalTimeSpan.GetStart();
		FbxTime endTime = takeInfo->mLocalTimeSpan.GetStop();

		FbxTime difference = endTime - startTime;

		float test = difference.GetSecondDouble();

		int nSkinDeformers = pfbxMesh->GetDeformerCount(FbxDeformer::eSkin);

		FbxAMatrix keyFrameMatrix;

		for (int i = 0; i < nSkinDeformers; i++)
		{
			FbxSkin* pfbxSkinDeformer = (FbxSkin*)pfbxMesh->GetDeformer(i, FbxDeformer::eSkin);
			int nClusters = pfbxSkinDeformer->GetClusterCount();

			bool flag{ true };
			if (nClusters > 0)
			{
				for (int i{}; i < boneNameList.size(); ++i)
				{
					if (nClusters != boneNameList.size())
					{
						flag = false;
						break;
					}
					if (boneNameList[i] != pfbxSkinDeformer->GetCluster(i)->GetLink()->GetName())
						flag = false;
				}

				if (!flag)
					break;

				int lerpCount = difference.GetSecondDouble() * 60;


				for (int j = 0; j < nClusters; j++)
				{
					FbxCluster* pfbxCluster = pfbxSkinDeformer->GetCluster(j);
					if (!pfbxCluster->GetLink()) continue;

					amazing.emplace_back();

					for (int l{}; l < lerpCount; ++l)
					{
						keyFrameMatrix = pfbxCluster->GetLink()->EvaluateGlobalTransform(startTime + (difference / lerpCount) * l);
						amazing.back().push_back(keyFrameMatrix);
					}
				}
			}
		}
	}

	int nChilds = startNode->GetChildCount();
	for (int i{}; i < nChilds; ++i)
	{
		GetKeyFrameMatrixGlobal(startNode->GetChild(i), boneNameList, amazing);
	}
}

void DisplayHierachy::GetKeyFrameMatrixLocal(FbxNode* startNode, const std::vector<std::vector<std::string>>& boneNameList, std::vector<std::vector< std::vector<FbxAMatrix>>>& amazing)
{
	int nChilds = startNode->GetChildCount();

	for (int j{}; j < boneNameList.size(); ++j)
	{
		amazing.emplace_back();

		for (int i{}; i < nChilds; ++i)
		{
			GetKeyFrameMatrixLocal(startNode->GetChild(i), boneNameList[j], amazing.back());
		}
	}
}


void DisplayHierachy::GetKeyFrameMatrixLocal(FbxNode* startNode, const std::vector<std::string>& boneNameList, std::vector< std::vector<FbxAMatrix>>& amazing)
{
	FbxNodeAttribute* pfbxNodeAttribute = startNode->GetNodeAttribute();
	if (pfbxNodeAttribute && (pfbxNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh))
	{
		FbxMesh* pfbxMesh = startNode->GetMesh();

		int nSkinDeformers = pfbxMesh->GetDeformerCount(FbxDeformer::eSkin);

		for (int i = 0; i < nSkinDeformers; i++)
		{
			FbxSkin* pfbxSkinDeformer = (FbxSkin*)pfbxMesh->GetDeformer(i, FbxDeformer::eSkin);
			int nClusters = pfbxSkinDeformer->GetClusterCount();

			bool flag{ true };
			if (nClusters > 0)
			{
				for (int i{}; i < boneNameList.size(); ++i)
				{
					if (nClusters != boneNameList.size())
					{
						flag = false;
						break;
					}
					if (boneNameList[i] != pfbxSkinDeformer->GetCluster(i)->GetLink()->GetName())
						flag = false;
				}

				if (!flag)
					break;

				for (int j = 0; j < nClusters; j++)
				{
					FbxCluster* pfbxCluster = pfbxSkinDeformer->GetCluster(j);
					if (!pfbxCluster->GetLink()) continue;

					amazing.emplace_back();
					//std::cout << pfbxCluster->GetLink()->GetName() << std::endl;
					//std::cout << amazing.back().size() << std::endl;

					FbxAMatrix keyFrameMatrix = pfbxCluster->GetLink()->EvaluateLocalTransform(0);
					//keyFrameMatrix.SetIdentity();
					amazing.back().push_back(keyFrameMatrix);
					//OutputDebugStringA(pfbxCluster->GetLink()->GetName());
					//OutputDebugStringW(std::to_wstring(keyFrameMatrix.mData[3][0]).c_str());
					//OutputDebugStringW(std::to_wstring(keyFrameMatrix.mData[3][1]).c_str());
					//OutputDebugStringW(std::to_wstring(keyFrameMatrix.mData[3][2]).c_str());



					for (int l{}; l < 90; ++l)
					{
						keyFrameMatrix = pfbxCluster->GetLink()->EvaluateLocalTransform((l + 1) * 10000000);
						amazing.back().push_back(keyFrameMatrix);
					}


					//OutputDebugStringW(std::to_wstring(amazing.back().size()).c_str());

				}
			}
		}
	}





	int nChilds = startNode->GetChildCount();
	for (int i{}; i < nChilds; ++i)
	{
		GetKeyFrameMatrixLocal(startNode->GetChild(i), boneNameList, amazing);
	}


}

void DisplayHierachy::GetMaterialGroups(FbxNode* startNode, std::vector<std::vector<PrimitiveGroup>>& primitiveGroups, const std::string fileName)
{
	FbxNodeAttribute* pfbxNodeAttribute = startNode->GetNodeAttribute();
	if (pfbxNodeAttribute)
	{
		FbxNodeAttribute::EType nAttributeType = pfbxNodeAttribute->GetAttributeType();
		if (nAttributeType == FbxNodeAttribute::eMesh)
		{
			FbxMesh* pfbxMesh = (FbxMesh*)startNode->GetNodeAttribute();



			FbxLayerElementArrayTemplate<int>* arr;
			pfbxMesh->GetMaterialIndices(&arr);
			//FbxGeometryElementUV* uv = pfbxMesh->GetElementUV()
			FbxGeometryElementUV* uv = pfbxMesh->GetElementUV(0);



			FbxLayer* layer = pfbxMesh->GetLayer(0);
			FbxLayerElementMaterial* mat = layer->GetMaterials();

			if (mat)
			{
				mat->SetMappingMode(FbxLayerElement::EMappingMode::eByPolygon);
				mat->SetReferenceMode(FbxLayerElement::EReferenceMode::eIndexToDirect);
				auto& indexArray = mat->GetIndexArray();



				//std::cout << "임 : " << indexArray.GetCount() << std::endl;
				//std::ofstream out("materialIndex.txt");
				//for (int i{}; i < indexArray.GetCount(); ++i)
				//{
				//	out << i << " 번 폴리곤의 머터리얼 인덱스 : " << indexArray[i] << std::endl;
				//}


				primitiveGroups.emplace_back();


				//std::ofstream out2(fileName + ".hag4");
				int temp{};
				int count{};
				int start{};
				//int end{};
				for (int i{}; i < indexArray.GetCount(); ++i)
				{
					if (i == 0)
					{
						temp = indexArray[i];
					}

					if (temp != indexArray[i])
					{

						primitiveGroups.back().emplace_back();
						primitiveGroups.back().back().start = start;
						primitiveGroups.back().back().end = i - 1;
						primitiveGroups.back().back().materialIndex = temp;

						//out2 << "Group " << count++ << " : " << start << "~" << i - 1 << " Material : " << temp << std::endl;


						temp = indexArray[i];
						start = i;

					}


				}
				//out2 << "Group " << count++ << " : " << start << "~" << indexArray.GetCount() - 1 << " Material : " << temp << std::endl;
				primitiveGroups.back().emplace_back();
				primitiveGroups.back().back().start = start;
				primitiveGroups.back().back().end = indexArray.GetCount() - 1;
				primitiveGroups.back().back().materialIndex = temp;



			}

		}
	}

	int nChilds = startNode->GetChildCount();
	for (int i{}; i < nChilds; ++i)
	{

		GetMaterialGroups(startNode->GetChild(i), primitiveGroups, fileName);

	}

}