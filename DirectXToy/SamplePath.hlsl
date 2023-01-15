#include "Common.hlsl"

struct VSInput
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
};


struct VSOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	float2 TexC    : TEXCOORD;
	
	// nointerpolation is used so the index is not interpolated 
	// across the triangle.
	nointerpolation uint MatIndex  : MATINDEX;
};

VSOut VSMain(VSInput vin, uint instanceID : SV_InstanceID)
{
	VSOut vsOut;
	vsOut.PosH = float4(0, 0, 0, 0);

	return vsOut;
}

float4 PSMain(VSOut pin) : SV_Target
{
	return float4(1.0, 0.5, 0.2, 1.0);
}