
cbuffer cbPointLight : register(b0)
{
    float4 Ambient;
    float3 Diffuse;
    float padding;
    float3 Position;
    float padding2;
    float Attenuation;
    float Range;
};

struct PS_Input
{
    float4 Position         : SV_POSITION;
    float4 WorldPosition    : WORLD_POSITION;
    float2 TexCoord         : TEXCOORD;
    float3 Normal           : NORMAL;
    float3 Tangent          : TANGENT;
    float3 Bitangent        : BITANGENT;
};

Texture2D baseTexture       : register(t0);
Texture2D normalTexture     : register(t1);
Texture2D metallicTexture   : register(t2);
Texture2D emissiveTexture   : register(t3);

float4 PS(PS_Input pin) : SV_TARGET
{
    

    return float4();
}