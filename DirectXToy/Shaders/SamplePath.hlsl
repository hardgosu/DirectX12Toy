
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

    vout.PosH = float4(vin.PosL, 1.0f);
	return vout;
}

float4 PSMain(VSOut pin) : SV_Target
{

    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}