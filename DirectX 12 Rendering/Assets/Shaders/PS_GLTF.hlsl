
struct PS_Input
{
    float4 Position         : SV_POSITION;
    float4 WorldPosition    : WORLD_POSITION;
    float2 TexCoord         : TEXCOORD;
    float3 Normal           : NORMAL;
    float3 Tangent          : TANGENT;
    float3 Bitangent        : BITANGENT;
};

Texture2D baseTexture           : register(t0);
Texture2D normalTexture         : register(t1);
Texture2D metallicTexture       : register(t2);
Texture2D emissiveTexture       : register(t3);

SamplerState baseSampler        : register(s0);

float4 PS(PS_Input pin) : SV_TARGET
{
    return float4(0.5f, 0.5f, 0.5f, 1.0f);
    //return float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    //if (baseTexture.Sample(baseSampler, pin.TexCoord).a < 0.1f)
    //   discard;
    
    //return float4(baseTexture.Sample(baseSampler, pin.TexCoord));
}