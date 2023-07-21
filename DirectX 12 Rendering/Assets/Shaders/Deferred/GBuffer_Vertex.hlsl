#ifndef GBUFFER_VERTEX_HLSL
#define GBUFFER_VERTEX_HLSL

#include "GBuffer.hlsli"

cbuffer cbPerObject : register(b0, space0)
{
    row_major float4x4 WVP;
    row_major float4x4 World;
}

cbuffer cbCamera : register(b1, space0)
{
    float4   CameraPosition;
    float4x4 View;
    float4x4 Projection;
    float4x4 InversedView;
    float4x4 InversedProjection;
    float2   ScreenDimension;
}

// add material here

DeferredOutput main(DeferredInput vin)
{
    DeferredOutput output = (DeferredOutput) 0;
    output.Position = mul(WVP, float4(vin.Position, 1.0f));
    //output.WorldPosition = mul(World * View, float4(vin.Position, 1.0f));
    output.WorldPosition = output.Position / output.Position.w;
    output.TexCoord     = vin.TexCoord;
    output.Normal       = normalize(mul((float3x3) World, vin.Normal));
    output.Tangent      = normalize(mul((float3x3) World, vin.Tangent));
    output.Bitangent    = normalize(mul((float3x3) World, vin.Bitangent));
    
    output.ViewDirection = normalize((CameraPosition.xyz - output.WorldPosition.xyz));
    
    return output;
}

#endif //GBUFFER_VERTEX_HLSL
