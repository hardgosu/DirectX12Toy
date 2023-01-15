#ifndef DataHLSL
#define DataHLSL

#define MaxLights 16

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif


struct InstanceData
{
	float4x4 World;
	float4x4 TexTransform;
	uint     MaterialIndex;
	uint     InstPad0;
	uint     InstPad1;
	uint     InstPad2;
};

struct MaterialData
{
	float4   DiffuseAlbedo;
    float3   FresnelR0;
    float    Roughness;
	float4x4 MatTransform;
	uint     DiffuseMapIndex;
	uint     MatPad0;
	uint     MatPad1;
	uint     MatPad2;
};

//CPU에서 정의한 구조에 대응하는 캡슐
struct MaterialData2
{
	float4 DiffuseAlbedo;
	float3 FresnelR0;
	float Roughness;
	float4x4 MaterialTransform;
	uint normalMapIndex;
	uint diffuseMapIndex;
	uint specularMapIndex;	
	uint roughnesMapIndex;
};
//조명계산에 쓰일 캡슐
struct Material
{
	float4 DiffuseAlbedo;
	float3 FresnelR0;
	float Shininess;
};


struct Light
{
	float3 Strength;
	float FalloffStart; // point/spot light only
	float3 Direction;   // directional/spot light only
	float FalloffEnd;   // point/spot light only
	float3 Position;    // point light only
	float SpotPower;    // spot light only
};


#endif //DataHLSL