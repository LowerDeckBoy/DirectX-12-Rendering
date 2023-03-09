
struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 UV : TexCoord;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
};

Texture2D tex : register(t0);
SamplerState samplerState : register(s0);

float4 PS(PS_Input pin) : SV_TARGET
{

    return float4(tex.Sample(samplerState, pin.UV));
}