#ifndef SHADOWS_PS_HLSL
#define SHADOWS_PS_HLSL

struct PS_INPUT
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

cbuffer cbMaterial : register(b0, space1)
{
    float3 CameraPosition;
    float padding3;

    float4 BaseColorFactor;
    float4 EmissiveFactor;

    float MetallicFactor;
    float RoughnessFactor;
    float AlphaCutoff;
    bool bDoubleSided;
};

cbuffer cbLights : register(b1, space1)
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


ConstantBuffer<MaterialIndices> Indices : register(b0, space2);
Texture2D<float4> TexturesTable[]       : register(t0, space1);
SamplerState texSampler                 : register(s0, space0);

//Texture2D<float4> BaseColorTexture : register(t0, space2);
//Texture2D<float4> NormalTexture : register(t1, space2);
//Texture2D<float4> MetalRoughnessTexture : register(t2, space2);
//Texture2D<float4> EmissiveTexture : register(t3, space2);
//Texture2D<float4> PositionTexture : register(t4, space2);
Texture2D<float4> DepthTexture : register(t4, space0);

//TextureCube SkyTexture : register(t6, space2);
//TextureCube IrradianceTexture : register(t7, space2);
//TextureCube SpecularTexture : register(t8, space2);
//Texture2D<float4> SpecularBRDFTexture : register(t9, space2);

// https://github.com/matt77hias/RasterTek/tree/master/Tutorial%2040_Shadow%20Mapping/Engine
// https://rastertek.com/dx11tut40.html
// https://rastertek.com/dx11tut41.html
// https://github.com/CHCTW/DirectX12-Framework-/blob/master/GraphicsTechniques/ShadowMap/ShadowMap.cpp

float4 main(PS_INPUT pin) : SV_TARGET
{
    float bias = 0.001f;
    
    float4 baseColor;
    if (Indices.BaseColorIndex >= 0)
    {
        baseColor = TexturesTable[Indices.BaseColorIndex].Sample(texSampler, pin.TexCoord);
        clip(baseColor.a - 0.2f);
        
        baseColor *= BaseColorFactor;
    }
    
    if (Indices.NormalIndex >= 0)
    {
        float3 normalMap = normalize(2.0f * TexturesTable[Indices.NormalIndex].Sample(texSampler, pin.TexCoord).rgb - 1.0f);
        float3 tangent = normalize(pin.Tangent - dot(pin.Tangent, pin.Normal) * pin.Normal);
        float3 bitangent = cross(pin.Normal, tangent);
        float3x3 texSpace = float3x3(tangent, bitangent, pin.Normal);
        pin.Normal = normalize(mul(normalMap.xyz, texSpace));
    }
    
    
    float3 output = baseColor.rgb;
    //float3 output = float3(0.0f, 0.0f, 0.0f);
    float2 projectTexCoord = float2(0.0f, 0.0f);
    projectTexCoord.x =  pin.LightViewProjection.x / pin.LightViewProjection.w / 2.0f + 0.5f;
    projectTexCoord.y = -pin.LightViewProjection.y / pin.LightViewProjection.w / 2.0f + 0.5f;
    
    if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
    {
        // Sample the shadow map depth value from the depth texture using the sampler at the projected texture coordinate location.
        //float depth = DepthTexture.Sample(texSampler, projectTexCoord).r;
        float depth = pin.DepthMap.r * DepthTexture.Sample(texSampler, projectTexCoord).r;

		// Calculate the depth of the light.
        float lightDepthValue = pin.LightViewProjection.z / pin.LightViewProjection.w;

		// Subtract the bias from the lightDepthValue.
        lightDepthValue = lightDepthValue - bias;

          // Calculate the amount of light on this pixel.
        float lightIntensity = saturate(dot(pin.Normal, pin.LightPosition));

        if (lightDepthValue < depth)
        {
            if (lightIntensity > 0.0f)
            {
				// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
                output += (LightColors[0].rgb * lightIntensity);
            }
        }
    }
    output = saturate(output);
    output *= baseColor.rgb;
    float depth = (pin.DepthMap.r + DepthTexture.Sample(texSampler, projectTexCoord).r);
    return float4(depth, depth, depth, 1.0f);
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

#endif // SHADOWS_PS_HLSL
