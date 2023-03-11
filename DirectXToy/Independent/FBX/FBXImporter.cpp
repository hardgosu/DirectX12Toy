#include "FBXImporter.h"

void FBXImporter::BuildFBXRenderItem(std::string filePath, OffsetOption offsetOption, Capsule* capsule)
{

	memberAnimationClipNameArray.Clear();
	memberStartTimes.clear();
	memberEndTimes.clear();
	memberNumberAnimationStack = 0;


	DisplayHierachy displayHierachy;
	FbxManager* pfbxSdkManager = NULL;
	FbxScene* pfbxScene = NULL;

	InitializeSdkObjects(pfbxSdkManager, pfbxScene);

	FbxString lFilePath(filePath.c_str());

	bool bResult = LoadScene(pfbxSdkManager, pfbxScene, lFilePath.Buffer());

	std::string fileName = filePath.substr(0,filePath.find('.', 0));


	std::vector<std::vector<std::vector<FbxAMatrix>>> allKeyFrames;

	

	

	if (bResult == false)
	{
		return;
	}
	else
	{
		FbxGeometryConverter fbxGeomConverter(pfbxSdkManager);
		fbxGeomConverter.Triangulate(pfbxScene, true);
		//fbxGeomConverter.RemoveBadPolygonsFromMeshes(pfbxScene, NULL);
		//fbxGeomConverter.SplitMeshesPerMaterial(pfbxScene, true);
		
		FbxAxisSystem fbxSceneAxisSystem = pfbxScene->GetGlobalSettings().GetAxisSystem();
		FbxAxisSystem fbxDirectXAxisSystem(FbxAxisSystem::eOpenGL);

		//if (fbxSceneAxisSystem != fbxDirectXAxisSystem)
		fbxDirectXAxisSystem.ConvertScene(pfbxScene);


		FbxSystemUnit fbxSceneSystemUnit = pfbxScene->GetGlobalSettings().GetSystemUnit();
		FbxSystemUnit::m.ConvertScene(pfbxScene);


		//printf_s("<GlobalTimeSettings>\n");
  //      DisplayGlobalTimeSettings(&pfbxScene->GetGlobalSettings());
		//printf_s("</GlobalTimeSettings>\n");
		pfbxScene->FillAnimStackNameArray(memberAnimationClipNameArray);
		if (displayHierachy.GetSkinnedMeshCount(pfbxScene->GetRootNode(), true))
		{

			displayHierachy.GetBoneNames(pfbxScene->GetRootNode(), capsule->memberBoneNameList);
			displayHierachy.GetMeshes(pfbxScene->GetRootNode(), capsule->vertices, capsule->indices, true,false);

			//스킨 메쉬 정점버퍼 집합만 넘겨주는것이 제일 좋다.
			displayHierachy.GetOffsets(pfbxScene->GetRootNode(), capsule->vertices, capsule->memberBoneOffsets, true);



			memberNumberAnimationStack = memberAnimationClipNameArray.GetCount();

			for (int i{}; i < memberNumberAnimationStack; ++i)
			{
				SetCurrentAnimationStack(pfbxScene, i);

				if (offsetOption == OffsetOption::Global)
				{
					displayHierachy.GetKeyFrameMatrixGlobal(pfbxScene->GetRootNode(), capsule->memberBoneNameList, allKeyFrames);
				}
				else
				{
					displayHierachy.GetHierarchies(pfbxScene->GetRootNode(), capsule->memberBoneNameList, capsule->memberHierarchies);
					displayHierachy.GetKeyFrameMatrixLocal(pfbxScene->GetRootNode(), capsule->memberBoneNameList, allKeyFrames);
				}
				capsule->animationClipNames.emplace_back(memberAnimationClipNameArray[i]->Buffer());
			}


			//ProgrammingUtility::PUOutputDebugStringW(L"sad");
//ProgrammingUtility::PUOutputDebugStringW(allKeyFrames[0][52].size());

//#주의
			for (int k{}; k < allKeyFrames.size(); ++k)
			{
				capsule->memberBoneKeyFrames.emplace_back();
				capsule->memberBoneKeyFrames[k].resize(allKeyFrames[k].size());

				for (int j{}; j < capsule->memberBoneKeyFrames[k].size(); ++j)
				{
					for (int i{}; i < allKeyFrames[k][j].size(); ++i)
					{
						FbxAMatrix test = allKeyFrames[k][j][i];

						DirectX::XMFLOAT4X4 a
						{
							(float)test.mData[0][0],(float)test.mData[0][1],(float)test.mData[0][2],(float)test.mData[0][3],
							(float)test.mData[1][0],(float)test.mData[1][1],(float)test.mData[1][2],(float)test.mData[1][3],
							(float)test.mData[2][0],(float)test.mData[2][1],(float)test.mData[2][2],(float)test.mData[2][3],
							(float)test.mData[3][0],(float)test.mData[3][1],(float)test.mData[3][2],(float)test.mData[3][3],


						};
						//GetMatrixTXTFile(std::string("test") + std::to_string(i),a);
						capsule->memberBoneKeyFrames[k][j].keyFrameMatrixs.push_back(std::move(a));
					}

					//클립의 시작과 끝 기록.
					capsule->memberBoneKeyFrames[k][j].startTime = memberStartTimes[k];
					capsule->memberBoneKeyFrames[k][j].endTime = memberEndTimes[k];
				}
			}









		}
		else if (memberAnimationClipNameArray.GetCount())
		{

			displayHierachy.GetMeshes(pfbxScene->GetRootNode(), capsule->vertices, capsule->indices, false,false);

			memberNumberAnimationStack = memberAnimationClipNameArray.GetCount();
			for (int i{}; i < memberNumberAnimationStack; ++i)
			{
				FbxString* pfbxStackName = memberAnimationClipNameArray[i];
				FbxAnimStack* pfbxAnimationStack = pfbxScene->FindMember<FbxAnimStack>(pfbxStackName->Buffer());
				capsule->animationClipNames.emplace_back(memberAnimationClipNameArray[i]->Buffer());
				SetCurrentAnimationStack(pfbxScene,i);
				
				displayHierachy.GetKeyFrameMatrixGlobalNonSkinned(pfbxScene->GetRootNode(), allKeyFrames);
			}

			for (int k{}; k < allKeyFrames.size(); ++k)
			{
				capsule->memberBoneKeyFrames.emplace_back();
				capsule->memberBoneKeyFrames[k].resize(allKeyFrames[k].size());

				for (int j{}; j < capsule->memberBoneKeyFrames[k].size(); ++j)
				{
					for (int i{}; i < allKeyFrames[k][j].size(); ++i)
					{
						FbxAMatrix test = allKeyFrames[k][j][i];

						DirectX::XMFLOAT4X4 a
						{
							(float)test.mData[0][0],(float)test.mData[0][1],(float)test.mData[0][2],(float)test.mData[0][3],
							(float)test.mData[1][0],(float)test.mData[1][1],(float)test.mData[1][2],(float)test.mData[1][3],
							(float)test.mData[2][0],(float)test.mData[2][1],(float)test.mData[2][2],(float)test.mData[2][3],
							(float)test.mData[3][0],(float)test.mData[3][1],(float)test.mData[3][2],(float)test.mData[3][3],


						};
						//GetMatrixTXTFile(std::string("test") + std::to_string(i),a);
						capsule->memberBoneKeyFrames[k][j].keyFrameMatrixs.push_back(std::move(a));
					}

					//클립의 시작과 끝 기록.
					capsule->memberBoneKeyFrames[k][j].startTime = memberStartTimes[k];
					capsule->memberBoneKeyFrames[k][j].endTime = memberEndTimes[k];
				}
			}
		}
		else
		{
			displayHierachy.GetMeshes(pfbxScene->GetRootNode(), capsule->vertices,capsule->indices, false,true);

		}

		displayHierachy.GetMaterialGroups(pfbxScene->GetRootNode(), capsule->primitiveGroups, fileName);
	}





	//MessageBoxW(NULL, std::to_wstring(memberHierarchies[0].size()).c_str(), L"asd", 0);
	//MessageBoxW(NULL, std::to_wstring(memberHierarchies[1].size()).c_str(), L"asd", 0);


	

	DestroySdkObjects(pfbxSdkManager, bResult);

}
FBXImporter::~FBXImporter()
{

	FbxArrayDelete(memberAnimationClipNameArray);
}

bool FBXImporter::SetCurrentAnimationStack(FbxScene* pfbxScene,int index)
{


	if ((memberNumberAnimationStack == 0) || (index >= memberNumberAnimationStack)) return(false);





	FbxString* pfbxStackName = memberAnimationClipNameArray[index];

	FbxAnimStack* pfbxAnimationStack = pfbxScene->FindMember<FbxAnimStack>(pfbxStackName->Buffer());
	if (!pfbxAnimationStack) return(false);

	FbxAnimLayer* currentAnimationLayer{};

	currentAnimationLayer = pfbxAnimationStack->GetMember<FbxAnimLayer>();
	pfbxScene->SetCurrentAnimationStack(pfbxAnimationStack);

	FbxTakeInfo* pfbxTakeInfo = pfbxScene->GetTakeInfo(*pfbxStackName);

	if (pfbxTakeInfo)
	{
		memberEndTimes.emplace_back();
		memberStartTimes.emplace_back();


		memberEndTimes.back() = pfbxTakeInfo->mLocalTimeSpan.GetStop().GetSecondDouble();
		memberStartTimes.back() = pfbxTakeInfo->mLocalTimeSpan.GetStart().GetSecondDouble();
		

	}

	

	return true;

}