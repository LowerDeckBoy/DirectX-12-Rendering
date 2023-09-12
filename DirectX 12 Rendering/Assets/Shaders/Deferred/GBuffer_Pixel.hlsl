#ifndef GBUFFER_PIXEL_HLSL
#define GBUFFER_PIXEL_HLSL

#include "GBuffer.hlsli"

cbuffer cbMaterial : register(b0, space1)
{
    float3 CameraPos;
    float padding3;

    float4 BaseColorFactor;
    float4 EmissiveFactor;

    float MetallicFactor;
    float RoughnessFactor;
    float AlphaCutoff;
    bool  bDoubleSided;

};

GBuffer_Output main(DeferredOutput pin)
{
    GBuffer_Output output = (GBuffer_Output) 0;

    if (Indices.BaseColorIndex >= 0)
    {
        output.Albedo = bindless_textures[Indices.BaseColorIndex].Sample(texSampler, pin.TexCoord);
       
        if (output.Albedo.a < AlphaCutoff)
            clip(output.Albedo.a - AlphaCutoff);

        output.Albedo = output.Albedo;
    }
    else
    {
        output.Albedo = float4(0.5f, 0.5f, 0.5f, 1.0f);
    }
    
    if (Indices.NormalIndex >= 0)
    {
        float4 normalMap = normalize(2.0f * bindless_textures[Indices.NormalIndex].Sample(texSampler, pin.TexCoord) - 1.0f);
        float3 tangent = normalize(pin.Tangent - dot(pin.Tangent, pin.Normal) * pin.Normal);
        float3 bitangent = cross(pin.Normal, tangent);
        float3x3 texSpace = float3x3(tangent, bitangent, pin.Normal);
        output.Normal = float4(normalize(mul(normalMap.xyz, texSpace)), normalMap.w);
    }
    else
    {
        output.Normal = float4(pin.Normal, 1.0f);
    }
    
    if (Indices.MetallicRoughnessIndex >= 0)
    {
        float4 metallic = bindless_textures[Indices.MetallicRoughnessIndex].Sample(texSampler, pin.TexCoord);
        metallic.b *= MetallicFactor;
        metallic.g *= RoughnessFactor;
        output.Metallic = metallic;
    }
    else
    {
        output.Metallic = float4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    if (Indices.EmissiveIndex >= 0)
    {
        output.Emissive = bindless_textures[Indices.EmissiveIndex].Sample(texSampler, pin.TexCoord) * EmissiveFactor;
    }
    else
    {
        output.Emissive = float4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    output.WorldPosition = pin.WorldPosition;
    
    float z = pin.Position.z / pin.Position.w;
    output.DepthMap = float4(z, z, z, 1.0f);

	return output;
}

#endif //GBUFFER_PIXEL_HLSL
