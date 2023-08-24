#ifndef SHADOWS_VS_HLSL
#define SHADOWS_VS_HLSL

//#include "Common.hlsli"

cbuffer cbPerObject : register(b0, space0)
{
    row_major float4x4 WVP;
    row_major float4x4 World;
};

//cbuffer CameraBuffer : register(b1)
//{
//    
//};

cbuffer MatrixBuffer : register(b1, space0)
{
    row_major float4x4 LightViewProjection;
    float4 LightPosition;
    float4 LightColor;
    //float Radius;
};

//cbuffer Lights : register(b2)
//{
//   
//};

struct VS_INPUT
{
    float3 Position     : POSITION;
    float2 TexCoord     : TEXCOORD;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
    float3 Bitangent    : BITANGENT;
};

struct VS_OUTPUT
{
    float4 Position             : SV_POSITION;
    float4 WorldPosition        : WORLD_POSITION;
    float4 DepthMap             : DEPTH_MAP;
    float2 TexCoord             : TEXCOORD;
    float3 Normal               : NORMAL;
    float3 Tangent              : TANGENT;
    float3 Bitangent            : BITANGENT;
    float4 LightViewProjection  : TEXCOORD1;
    float3 LightPosition        : TEXCOORD2;
};

// https://www.rastertek.com/dx11tut40.html
VS_OUTPUT main(VS_INPUT vin)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Position         = mul(WVP, float4(vin.Position, 1.0f));
    output.WorldPosition    = mul(World, float4(vin.Position, 1.0f));
    output.TexCoord         = vin.TexCoord;
    output.Normal           = normalize(mul((float3x3) World, vin.Normal));
    output.Tangent          = normalize(mul((float3x3) World, vin.Tangent));
    output.Bitangent        = normalize(mul((float3x3) World, vin.Bitangent));
    
    float depth = output.Position.z / output.Position.w;
    output.DepthMap = float4(depth, depth, depth, 1.0f);
    
    //
    //output.LightViewProjection = mul(float4(vin.Position, 1.0f), World);
    //output.LightViewProjection = mul(output.LightViewProjection, LightViewProjection);
    output.LightViewProjection = mul(World, float4(vin.Position, 1.0f));
    output.LightViewProjection = mul(LightViewProjection, output.LightViewProjection);
    //output.LightViewProjection = mul(LightViewProjection, float4(vin.Position, 1.0f));
    //output.LightViewPosition = mul(LightProjection, output.LightViewPosition);
    output.LightPosition       = normalize(LightPosition.xyz - output.WorldPosition.xyz);
    
    return output;
}

#endif // SHADOWS_VS_HLSL
