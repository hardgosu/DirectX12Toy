#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <D3dx9math.h>
#include <iostream>
#include <chrono>
#include <iomanip>

#pragma comment(lib, "D3dx9.lib")

using namespace DirectX;
using namespace DirectX::PackedVector;


bool Test1(XMFLOAT4X4& left, XMFLOAT4X4& right)
{
	XMFLOAT4X4 out;
	XMStoreFloat4x4(&out, XMMatrixMultiply(XMLoadFloat4x4(&left), XMLoadFloat4x4(&right)));

	return true;
}

bool Test2(D3DXMATRIX& left, D3DXMATRIX& right)
{
	D3DXMATRIX out;
	D3DXMatrixMultiply(&out, &left, &right);

	return true;
}

D3DXMATRIX* D3DXMatrixInverse2(D3DXMATRIX* pout, FLOAT* pdeterminant, CONST D3DXMATRIX* pm);
bool Test3(D3DXMATRIX& left, CONST D3DXMATRIX& right)
{

	return true;
}

D3DXMATRIX* D3DXMatrixInverse2(D3DXMATRIX* pout, FLOAT* pdeterminant, CONST D3DXMATRIX* pm)
{
	FLOAT det, t[3], v[16];
	UINT i, j;
	t[0] = pm->m[2][2] * pm->m[3][3] - pm->m[2][3] * pm->m[3][2];
	t[1] = pm->m[1][2] * pm->m[3][3] - pm->m[1][3] * pm->m[3][2];
	t[2] = pm->m[1][2] * pm->m[2][3] - pm->m[1][3] * pm->m[2][2];
	v[0] = pm->m[1][1] * t[0] - pm->m[2][1] * t[1] + pm->m[3][1] * t[2];
	v[4] = -pm->m[1][0] * t[0] + pm->m[2][0] * t[1] - pm->m[3][0] * t[2];

	t[0] = pm->m[1][0] * pm->m[2][1] - pm->m[2][0] * pm->m[1][1];
	t[1] = pm->m[1][0] * pm->m[3][1] - pm->m[3][0] * pm->m[1][1];
	t[2] = pm->m[2][0] * pm->m[3][1] - pm->m[3][0] * pm->m[2][1];
	v[8] = pm->m[3][3] * t[0] - pm->m[2][3] * t[1] + pm->m[1][3] * t[2];
	v[12] = -pm->m[3][2] * t[0] + pm->m[2][2] * t[1] - pm->m[1][2] * t[2];

	det = pm->m[0][0] * v[0] + pm->m[0][1] * v[4] +
		pm->m[0][2] * v[8] + pm->m[0][3] * v[12];
	if (det == 0.0f)
		return NULL;
	if (pdeterminant)
		*pdeterminant = det;

	t[0] = pm->m[2][2] * pm->m[3][3] - pm->m[2][3] * pm->m[3][2];
	t[1] = pm->m[0][2] * pm->m[3][3] - pm->m[0][3] * pm->m[3][2];
	t[2] = pm->m[0][2] * pm->m[2][3] - pm->m[0][3] * pm->m[2][2];
	v[1] = -pm->m[0][1] * t[0] + pm->m[2][1] * t[1] - pm->m[3][1] * t[2];
	v[5] = pm->m[0][0] * t[0] - pm->m[2][0] * t[1] + pm->m[3][0] * t[2];

	t[0] = pm->m[0][0] * pm->m[2][1] - pm->m[2][0] * pm->m[0][1];
	t[1] = pm->m[3][0] * pm->m[0][1] - pm->m[0][0] * pm->m[3][1];
	t[2] = pm->m[2][0] * pm->m[3][1] - pm->m[3][0] * pm->m[2][1];
	v[9] = -pm->m[3][3] * t[0] - pm->m[2][3] * t[1] - pm->m[0][3] * t[2];
	v[13] = pm->m[3][2] * t[0] + pm->m[2][2] * t[1] + pm->m[0][2] * t[2];

	t[0] = pm->m[1][2] * pm->m[3][3] - pm->m[1][3] * pm->m[3][2];
	t[1] = pm->m[0][2] * pm->m[3][3] - pm->m[0][3] * pm->m[3][2];
	t[2] = pm->m[0][2] * pm->m[1][3] - pm->m[0][3] * pm->m[1][2];
	v[2] = pm->m[0][1] * t[0] - pm->m[1][1] * t[1] + pm->m[3][1] * t[2];
	v[6] = -pm->m[0][0] * t[0] + pm->m[1][0] * t[1] - pm->m[3][0] * t[2];

	t[0] = pm->m[0][0] * pm->m[1][1] - pm->m[1][0] * pm->m[0][1];
	t[1] = pm->m[3][0] * pm->m[0][1] - pm->m[0][0] * pm->m[3][1];
	t[2] = pm->m[1][0] * pm->m[3][1] - pm->m[3][0] * pm->m[1][1];
	v[10] = pm->m[3][3] * t[0] + pm->m[1][3] * t[1] + pm->m[0][3] * t[2];
	v[14] = -pm->m[3][2] * t[0] - pm->m[1][2] * t[1] - pm->m[0][2] * t[2];

	t[0] = pm->m[1][2] * pm->m[2][3] - pm->m[1][3] * pm->m[2][2];
	t[1] = pm->m[0][2] * pm->m[2][3] - pm->m[0][3] * pm->m[2][2];
	t[2] = pm->m[0][2] * pm->m[1][3] - pm->m[0][3] * pm->m[1][2];
	v[3] = -pm->m[0][1] * t[0] + pm->m[1][1] * t[1] - pm->m[2][1] * t[2];
	v[7] = pm->m[0][0] * t[0] - pm->m[1][0] * t[1] + pm->m[2][0] * t[2];

	v[11] = -pm->m[0][0] * (pm->m[1][1] * pm->m[2][3] - pm->m[1][3] * pm->m[2][1]) +
		pm->m[1][0] * (pm->m[0][1] * pm->m[2][3] - pm->m[0][3] * pm->m[2][1]) -
		pm->m[2][0] * (pm->m[0][1] * pm->m[1][3] - pm->m[0][3] * pm->m[1][1]);

	v[15] = pm->m[0][0] * (pm->m[1][1] * pm->m[2][2] - pm->m[1][2] * pm->m[2][1]) -
		pm->m[1][0] * (pm->m[0][1] * pm->m[2][2] - pm->m[0][2] * pm->m[2][1]) +
		pm->m[2][0] * (pm->m[0][1] * pm->m[1][2] - pm->m[0][2] * pm->m[1][1]);

	det = 1.0f / det;

	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			pout->m[i][j] = v[4 * i + j] * det;

	return pout;
}

int main()
{
#ifdef _X86_
	std::cout << "X86" << std::endl;
#else
	std::cout << "X64" << std::endl;
#endif
	/*
		1,0,0,0,
		0,4,5,0,
		0,1,1,0,
		0,0,0,1
	*/
	XMFLOAT4X4 a
	{
		1,5,0,2,
		0,4,5,0,
		2,1,1,0,
		6,0,0,1
	};
	XMFLOAT4X4 b
	{
		1,0,0,0,
		0,4,5,0,
		0,1,1,0,
		0,0,0,1
	};

	D3DXMATRIX aa
	{
		1,5,0,2,
		0,4,5,0,
		2,1,1,0,
		6,0,0,1
	};
	D3DXMATRIX bb
	{
		1,0,0,0,
		0,4,5,0,
		0,1,1,0,
		0,0,0,1
	};

	auto begin = std::chrono::steady_clock::now();

	volatile long long loop = 500000000;
	for (long long i{}; i < loop; ++i)
	{
		Test1(a, b);
	}
	auto end = std::chrono::steady_clock::now();
	std::cout << "DX12 : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << std::endl;

	begin = std::chrono::steady_clock::now();
	for (long long i{}; i < loop; ++i)
	{
		Test2(aa, bb);
	}
	end = std::chrono::steady_clock::now();
	std::cout << "DX 9 : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << std::endl;

	/*
	begin = std::chrono::steady_clock::now();
	for (long long i{}; i < loop; ++i)
	{
		Test3(aa, bb);
	}
	end = std::chrono::steady_clock::now();
	std::cout << "CM   : " <<std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << std::endl;
	*/
}