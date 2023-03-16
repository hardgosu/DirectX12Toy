#include <DirectXMath.h>
#include <Windows.h>
#include <d3d12.h>
#include <ppl.h>
#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <chrono>
#include <atomic>
#include <string>
#include <unordered_map>
#include <concurrent_unordered_map.h>
#include <concurrent_vector.h>

constexpr int ThreadCount = 4;
volatile int Loop = 1000000 / ThreadCount;

using namespace DirectX;

struct Keyframe
{
	Keyframe()
		: TimePos(0.0f),
		Translation(0.0f, 0.0f, 0.0f),
		Scale(1.0f, 1.0f, 1.0f),
		RotationQuat(0.0f, 0.0f, 0.0f, 1.0f)
	{
	}

	float TimePos;
	DirectX::XMFLOAT3 Translation;
	DirectX::XMFLOAT3 Scale;
	DirectX::XMFLOAT4 RotationQuat;
};

struct BoneAnimation
{
	BoneAnimation()
	{
		Keyframes.resize(50);
	}
	void Interpolate(float t, XMFLOAT4X4& M) const
	{
		if (t <= Keyframes.front().TimePos)
		{
			XMVECTOR S = XMLoadFloat3(&Keyframes.front().Scale);
			XMVECTOR P = XMLoadFloat3(&Keyframes.front().Translation);
			XMVECTOR Q = XMLoadFloat4(&Keyframes.front().RotationQuat);

			XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
		}
		else if (t >= Keyframes.back().TimePos)
		{
			XMVECTOR S = XMLoadFloat3(&Keyframes.back().Scale);
			XMVECTOR P = XMLoadFloat3(&Keyframes.back().Translation);
			XMVECTOR Q = XMLoadFloat4(&Keyframes.back().RotationQuat);

			XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
		}
		else
		{
			for (UINT i = 0; i < Keyframes.size() - 1; ++i)
			{
				if (t >= Keyframes[i].TimePos && t <= Keyframes[i + 1].TimePos)
				{
					float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i + 1].TimePos - Keyframes[i].TimePos);

					XMVECTOR s0 = XMLoadFloat3(&Keyframes[i].Scale);
					XMVECTOR s1 = XMLoadFloat3(&Keyframes[i + 1].Scale);

					XMVECTOR p0 = XMLoadFloat3(&Keyframes[i].Translation);
					XMVECTOR p1 = XMLoadFloat3(&Keyframes[i + 1].Translation);

					XMVECTOR q0 = XMLoadFloat4(&Keyframes[i].RotationQuat);
					XMVECTOR q1 = XMLoadFloat4(&Keyframes[i + 1].RotationQuat);

					XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
					XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
					XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

					XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
					XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));

					break;
				}
			}
		}
	}

	std::vector<Keyframe> Keyframes;
};

struct AnimationClip
{
	AnimationClip()
	{
		BoneAnimations.resize(50);
	}

	void Interpolate(float t, std::vector<DirectX::XMFLOAT4X4>& boneTransforms)const
	{
		for (UINT i = 0; i < BoneAnimations.size(); ++i)
		{
			BoneAnimations[i].Interpolate(t, boneTransforms[i]);
		}
	}
	std::vector<BoneAnimation> BoneAnimations;
};



void GetFinalTransforms(volatile float timePos, std::vector<XMFLOAT4X4>& finalTransforms)
{
	// Gives parentIndex of ith bone.
	std::vector<int> mBoneHierarchy
	{
		0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
		21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
		32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
		43, 44, 45, 46, 47, 48, 49,
	};

	std::vector<DirectX::XMFLOAT4X4> mBoneOffsets(50);
	UINT numBones = mBoneOffsets.size();

	std::vector<XMFLOAT4X4> toParentTransforms(numBones);

	// Interpolate all the bones of this clip at the given time instance.
	AnimationClip clip;

	clip.Interpolate(timePos, toParentTransforms);

	//
	// Traverse the hierarchy and transform all the bones to the root space.
	//

	std::vector<XMFLOAT4X4> toRootTransforms(numBones);

	// The root bone has index 0.  The root bone has no parent, so its toRootTransform
	// is just its local bone transform.
	toRootTransforms[0] = toParentTransforms[0];

	// Now find the toRootTransform of the children.
	for (UINT i = 1; i < numBones; ++i)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);

		int parentIndex = mBoneHierarchy[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);

		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}

	// Premultiply by the bone offset transform to get the final transform.
	for (UINT i = 0; i < numBones; ++i)
	{
		XMMATRIX offset = XMLoadFloat4x4(&mBoneOffsets[i]);
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
		XMMATRIX finalTransform = XMMatrixMultiply(offset, toRoot);
		XMStoreFloat4x4(&finalTransforms[i], XMMatrixTranspose(finalTransform));
	}
}

using TestMap = concurrency::concurrent_vector <std::string>;
//using TestMap = std::vector<std::string>;
TestMap a;

std::mutex lock;

int main()
{
	{
		auto begin = std::chrono::high_resolution_clock::now();

		std::vector<std::thread> threads;
		for (int i{}; i < ThreadCount; ++i)
		{
			threads.emplace_back([threadNumber = i]()
				{
					for (int j{ Loop * threadNumber }; j < Loop * (threadNumber + 1); ++j)
					{
						//std::lock_guard<std::mutex> l(lock);
						a.push_back(std::to_string(j) + "asd");
					}
				});
		}
		std::for_each(threads.begin(), threads.end(), [](auto& thread) { thread.join(); });

		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "Time : "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;
	}
	a.clear();
	{
		auto begin = std::chrono::high_resolution_clock::now();
		concurrency::parallel_for(0, Loop * ThreadCount, [](int i)
			{
				//std::lock_guard<std::mutex> l(lock);
				a[i] = std::to_string(i) + "asd";
			});

		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "Time : "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;

	}
}
