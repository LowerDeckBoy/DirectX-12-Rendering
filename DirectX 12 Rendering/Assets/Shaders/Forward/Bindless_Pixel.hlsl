


#include "Lighting_PBR.hlsli"

cbuffer cbMaterial : register(b0, space0)
{
    float3 CameraPosition;
    float padding;

    float4 BaseColorFactor;
    float4 EmissiveFactor;

    float MetallicFactor;
    float RoughnessFactor;
    float AlphaCutoff;
    bool bDoubleSided;
};

cbuffer cbLights : register(b1, space0)
{
    float4 LightPositions[4];
    float4 LightColors[4];
    float lightsPadding[32];
}

struct MaterialIndices
{
    int BaseColorIndex;
    int NormalIndex;
    int MetallicRoughnessIndex;
    int EmissiveIndex;
};

ConstantBuffer<MaterialIndices> Indices : register(b1, space1);
Texture2D<float4> bindless_textures[] : register(t0, space1);
SamplerState texSampler : register(s0);

float4 main(PS_INPUT pin) : SV_TARGET
{
    float3 output = float3(0.0f, 0.0f, 0.0f);
    
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    if (Indices.BaseColorIndex >= 0)
    {
        diffuse = bindless_textures[Indices.BaseColorIndex].Sample(texSampler, pin.TexCoord);
        if (diffuse.a < AlphaCutoff)
            discard;
        
        output = diffuse.rgb;
    } 
    //float4 
    //float4 diffuse = bindless_textures.Sample(texSampler, pin.TexCoord);

    //return float4(pin.TexCoord, 0.0f, 1.0f);
    return float4(output, 1.0f);
}
