#ifndef RootSignature1HLSL
#define RootSignature1HLSL

#include "Data.hlsl"

// An array of textures, which is only supported in shader model 5.1+.  Unlike Texture2DArray, the textures
// in this array can be different sizes and formats, making it more flexible than texture arrays.
Texture2D gDiffuseMap[7] : register(t0);

// Put in space1, so the texture array does not overlap with these resources.  
// The texture array will occupy registers t0, t1, ..., t6 in space0. 
StructuredBuffer<InstanceData> gInstanceData : register(t0, space1);
StructuredBuffer<MaterialData> gMaterialData : register(t1, space1);

StructuredBuffer<MaterialData> gMaterials : register(t0);
Texture2D gShadowMap : register(t1);
Texture2D gDisplacementMap : register(t2);
Texture2D gTextureMap[40] : register(t4);
TextureCube gCubeMap : register(t3);

TextureCube gDynamicCubeMap : register(t44);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

//카메라의 정보를 위한 상수 버퍼를 선언한다.
cbuffer cbPass1 : register(b0)
{
	float4x4 gmtxView;
	float4x4 gmtxProjection;
	float4x4 gShadowMatrix;
	float3 gEyePosW;
	float empty;
	float gTotalTime;
    float gDeltaTime;
	float2 empty2;
	Light gLights[MaxLights];
	float4 gAmbientLight;
	float2 gRenderTargetSize;
	float2 gInverseRenderTargetSize;
};

// Constant data that varies per pass.
cbuffer cbPass2 : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3 gEyePosW_2;
    float cbPerObjectPad1;
    float2 gRenderTargetSize_2;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime_2;
    float gDeltaTime_2;
    float4 gAmbientLight_2;

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light gLights_2[MaxLights];
};

//인스턴스 데이터
cbuffer cbObjectInfo : register(b2)
{
	float4x4 gTextureTransform;
	int4x4 gStartPolygonIndice[2];
	int4x4 gEndPolygonIndice[2];
	int gMaterialIndex;
	float2 gDisplacementMapTexelSize;
	float gGridSpatialStep;
	int gTextureIndex;
	int gIsHeightMapped;
}

//그냥 파티클 전용 루트 시그니처 만드는게 나아
cbuffer cbParticle : register(b3)
{
    uint gParticleMaterialIndex;
    uint gParticleTextureIndex;
};

#endif //RootSignature1HLSL