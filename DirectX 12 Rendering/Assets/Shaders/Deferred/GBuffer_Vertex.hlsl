#ifndef GBUFFER_VERTEX_HLSL
#define GBUFFER_VERTEX_HLSL

#include "GBuffer.hlsli"

cbuffer cbPerObject : register(b0, space0)
{
    row_major float4x4 WVP;
    row_major float4x4 World;
}

DeferredOutput main(DeferredInput vin)
{
    DeferredOutput output   = (DeferredOutput) 0;
    output.Position         = mul(WVP, float4(vin.Position, 1.0f));
    output.WorldPosition    = mul(World, float4(vin.Position, 1.0f));
    output.TexCoord         = vin.TexCoord;
    output.Normal           = normalize(mul((float3x3) World, vin.Normal));
    output.Tangent          = normalize(mul((float3x3) World, vin.Tangent));
    output.Bitangent        = normalize(mul((float3x3) World, vin.Bitangent));

    return output;
}

#endif //GBUFFER_VERTEX_HLSL
