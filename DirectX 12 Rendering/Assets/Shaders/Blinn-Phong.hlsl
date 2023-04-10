
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

struct PS_INPUT
{
    float4 Position         : SV_POSITION;
    float4 WorldPosition    : WORLD_POSITION;
    float3 ViewDirection    : VIEW_DIRECTION;
    float2 TexCoord         : TEXCOORD;
    float3 Normal           : NORMAL;
    float3 Tangent          : TANGENT;
    float3 Bitangnent       : BITANGENT;    
};

Texture2D baseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metallicTexture : register(t2);
Texture2D emissiveTexture : register(t3);

SamplerState baseSampler : register(s0);

float4 PS(PS_INPUT pin) : SV_TARGET
{
    
    float3 lightDirection = normalize(Direction - pin.WorldPosition.xyz);
    float3 viewDirection = normalize(pin.ViewDirection - pin.WorldPosition.xyz);
    float3 halfway = normalize(lightDirection + viewDirection);
    
    
    
    return float4(0.0f, 0.0f, 0.0f, 1.0f);
}