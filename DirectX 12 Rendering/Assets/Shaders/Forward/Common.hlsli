#ifndef COMMON_HLSLI
#define COMMON_HLSLI

// 3D Object Matrices
cbuffer cbPerObject : register(b0, space0)
{
    row_major float4x4 WVP;
    row_major float4x4 World;
}

cbuffer cbCamera : register(b1, space0)
{
    float4 CameraPosition;
}

struct VS_Input
{
    float3 Position     : POSITION;
    float2 TexCoord     : TEXCOORD;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
    float3 Bitangent    : BITANGENT;
};

struct VS_Output
{
    float4 Position         : SV_POSITION;
    float4 WorldPosition    : WORLD_POSITION;
    float3 ViewDirection    : VIEW_DIRECTION;
    float2 TexCoord         : TEXCOORD;
    float3 Normal           : NORMAL;
    float3 Tangent          : TANGENT;
    float3 Bitangent        : BITANGENT;
};

#endif // COMMON_HLSLI
