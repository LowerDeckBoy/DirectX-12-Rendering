#ifndef POINT_SHADOWS_VS_HLSL
#define POINT_SHADOWS_VS_HLSL

//#include "Common.hlsli"

cbuffer cbPerObject : register(b0, space0)
{
    row_major float4x4 WVP;
    row_major float4x4 World;
};

struct VS_INPUT
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : WORLD_POSITION;
    float3 ViewDirection : VIEW_DIRECTION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
};

// https://www.rastertek.com/dx11tut41.html
VS_OUTPUT main(VS_INPUT vin)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Position = mul(WVP, float4(vin.Position, 1.0f));
    output.WorldPosition = mul(World, float4(vin.Position, 1.0f));
    output.TexCoord = vin.TexCoord;
    output.Normal = normalize(mul((float3x3) World, vin.Normal));
    output.Tangent = normalize(mul((float3x3) World, vin.Tangent));
    output.Bitangent = normalize(mul((float3x3) World, vin.Bitangent));

    return output;
}

#endif // POINT_SHADOWS_VS_HLSL
