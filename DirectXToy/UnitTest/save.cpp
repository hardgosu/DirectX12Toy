/*
void BoneAnimation::Interpolate(float t, XMFLOAT4X4& M)const
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
		float normalizedPercent = t / GetEndTime();

		size_t previousIndex = normalizedPercent * Keyframes.size() - 1;

		float lerpPercent = (t - Keyframes[previousIndex].TimePos) / (Keyframes[previousIndex + 1].TimePos - Keyframes[previousIndex].TimePos);

		XMVECTOR s0 = XMLoadFloat3(&Keyframes[previousIndex].Scale);
		XMVECTOR s1 = XMLoadFloat3(&Keyframes[previousIndex + 1].Scale);

		XMVECTOR p0 = XMLoadFloat3(&Keyframes[previousIndex].Translation);
		XMVECTOR p1 = XMLoadFloat3(&Keyframes[previousIndex + 1].Translation);

		XMVECTOR q0 = XMLoadFloat4(&Keyframes[previousIndex].RotationQuat);
		XMVECTOR q1 = XMLoadFloat4(&Keyframes[previousIndex + 1].RotationQuat);

		XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
		XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
		XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));

		/*
		for(UINT i = 0; i < Keyframes.size()-1; ++i)
		{
			if( t >= Keyframes[i].TimePos && t <= Keyframes[i+1].TimePos )
			{
				float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i+1].TimePos - Keyframes[i].TimePos);

				XMVECTOR s0 = XMLoadFloat3(&Keyframes[i].Scale);
				XMVECTOR s1 = XMLoadFloat3(&Keyframes[i+1].Scale);

				XMVECTOR p0 = XMLoadFloat3(&Keyframes[i].Translation);
				XMVECTOR p1 = XMLoadFloat3(&Keyframes[i+1].Translation);

				XMVECTOR q0 = XMLoadFloat4(&Keyframes[i].RotationQuat);
				XMVECTOR q1 = XMLoadFloat4(&Keyframes[i+1].RotationQuat);

				XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
				XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
				XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));

				break;
			}
		}
		*/
	}
}
*/