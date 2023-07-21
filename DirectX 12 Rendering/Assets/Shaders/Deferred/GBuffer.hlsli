#ifndef GBUFFER_HLSLI
#define GBUFFER_HLSLI

// Input - Output structs
struct DeferredInput
{
    float3 Position     : POSITION;
    float2 TexCoord     : TEXCOORD;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
    float3 Bitangent    : BITANGENT;
};

struct DeferredOutput
{
    float4 Position         : SV_POSITION;
    float4 WorldPosition    : WORLD_POSITION;
    float3 ViewDirection    : VIEW_DIRECTION;
    float2 TexCoord         : TEXCOORD;
    float3 Normal           : NORMAL;
    float3 Tangent          : TANGENT;
    float3 Bitangent        : BITANGENT;
};

struct GBuffer_Output
{
    float4 Albedo           : SV_Target0;
    float4 Normal           : SV_Target1;
    float4 Metallic         : SV_Target2;
    float4 Emissive         : SV_Target3;
    float4 WorldPosition    : SV_Target4;
};

// Textures 
struct MaterialIndices
{
    int BaseColorIndex;
    int NormalIndex;
    int MetallicRoughnessIndex;
    int EmissiveIndex;
};

ConstantBuffer<MaterialIndices> Indices : register(b0, space2);
Texture2D<float4> bindless_textures[] : register(t0, space1);
//Texture2D<float4> DepthTexture : register(t4, space0);

SamplerState texSampler : register(s0);


#endif // GBUFFER_HLSLI