
cbuffer cbMaterial : register(b0)
{
    float4 Ambient;
    float3 Diffuse;
    float padding;
    float3 Specular;
    float SpecularIntensity;
    float3 Direction;
    float padding2;
    float4 padding3[12];
};

struct PS_Input
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : WORLD_POSITION;
    float3 ViewDirection : VIEW_DIRECTION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
};

Texture2D baseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metallicTexture : register(t2);
Texture2D emissiveTexture : register(t3);

SamplerState baseSampler : register(s0);

float4 PS(PS_Input pin) : SV_TARGET
{
    float4 baseColor = baseTexture.Sample(baseSampler, pin.TexCoord) * Ambient;
    if (baseColor.a < 0.1f)
        discard;
    
    
    //float4 output = emissiveTexture.Sample(baseSampler, pin.TexCoord);
    float4 output = metallicTexture.Sample(baseSampler, pin.TexCoord);
    //float4 output = normalTexture.Sample(baseSampler, pin.TexCoord);
    
    return float4(output.rgb, 1.0f);
}