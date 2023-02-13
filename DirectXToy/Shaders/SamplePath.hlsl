#include "Common.hlsl"

struct VSInput
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentL : TANGENT;
};


struct VSOut
{
	float4 PosH    : SV_POSITION;
    float4 ShadowPosH : POSITION0;
    float4 SsaoPosH   : POSITION1;
    float3 PosW    : POSITION2;
    float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
	float2 TexC    : TEXCOORD;
    uint instanceID : INSTANCEID;
};

VSOut VSMain(VSInput vin, uint instanceID : SV_InstanceID)
{
	VSOut vout = (VSOut)0.0f;
	InstanceData instData = gInstanceData[instanceID];
	MaterialData matData = gMaterials[instData.MaterialIndex];
    // Transform to world space.
    float4 posW = mul(float4(vin.PosL, 1.0f), instData.WorldMatrix);
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
	// 비균등비례 하지마세요
    vout.NormalW = mul(vin.NormalL, (float3x3)instData.WorldMatrix);
	vout.TangentW = mul(vin.TangentL, (float3x3)instData.WorldMatrix);

    // Transform to homogeneous clip space.
    vout.PosH = mul(mul(posW, gViewMatrix), gProjectionMatrix);
	
	// Output vertex attributes for interpolation across triangle.
	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTextureMatrix);
	vout.TexC = mul(texC, matData.MaterialTransform).xy;

    // Generate projective tex-coords to project shadow map onto scene.
    vout.ShadowPosH = mul(posW, gShadowMatrix);
    vout.instanceID = instanceID;

	return vout;
}

float4 PSMain(VSOut pin) : SV_Target
{
	InstanceData instData = gInstanceData[pin.instanceID];
	// Fetch the material data.
	MaterialData matData = gMaterials[instData.MaterialIndex];
	float4 diffuseAlbedo = matData.DiffuseAlbedo;
	float3 fresnelR0 = matData.FresnelR0;
	float  roughness = matData.Roughness;
	uint diffuseMapIndex = matData.DiffuseMapIndex;
	uint normalMapIndex = matData.NormalMapIndex;
	
    // Dynamically look up the texture in the array.
    diffuseAlbedo *= gTextureMap[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);

#ifdef ALPHA_TEST
    // Discard pixel if texture alpha < 0.1.  We do this test as soon 
    // as possible in the shader so that we can potentially exit the
    // shader early, thereby skipping the rest of the shader code.
    clip(diffuseAlbedo.a - 0.1f);
#endif

	// Interpolating normal can unnormalize it, so renormalize it.
    pin.NormalW = normalize(pin.NormalW);
	
    float4 normalMapSample = gTextureMap[normalMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);

	// Uncomment to turn off normal mapping.
    //bumpedNormalW = pin.NormalW;

    // Vector from point being lit to eye. 
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

    // Only the first light casts a shadow.
    float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
    shadowFactor[0] = CalcShadowFactor(pin.ShadowPosH);

    const float shininess = (1.0f - roughness) * normalMapSample.a;
    Material mat = { diffuseAlbedo, fresnelR0, shininess };
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        bumpedNormalW, toEyeW, shadowFactor);

	float4 ambient = float4(0.2f, 0.2f, 0.2f, 0.2f);
    float4 litColor = ambient + directLight;

	// Add in specular reflections.
    float3 r = reflect(-toEyeW, bumpedNormalW);
    float4 reflectionColor = gCubeMap.Sample(gsamLinearWrap, r);
    float3 fresnelFactor = SchlickFresnel(fresnelR0, bumpedNormalW, r);
    litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
	
    // Common convention to take alpha from diffuse albedo.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}
