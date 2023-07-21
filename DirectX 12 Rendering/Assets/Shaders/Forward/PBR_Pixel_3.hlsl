#ifndef PBR_PIXEL_3_HLSL
#define PBR_PIXEL_3_HLSL

#include "Lighting_PBR.hlsli"

#define LIGHTS 4

cbuffer cbMaterial : register(b0)
{
    float3 CameraPosition;
    float padding3;

    float4 BaseColorFactor;
    float4 EmissiveFactor;

    float MetallicFactor;
    float RoughnessFactor;
    float AlphaCutoff;
    bool bDoubleSided;

    bool bHasDiffuse;
    bool bHasNormal;
    bool bHasMetallic;
    bool bHasEmissive;

    float4 padding5[8];
};

cbuffer cbLights : register(b1)
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
Texture2D<float4> TexturesTable[] : register(t0, space1);
SamplerState texSampler : register(s0);

Texture2D<float4> SkyTexture : register(t4, space0);

float4 main(PS_INPUT pin) : SV_TARGET
{
    float4 baseColor = TexturesTable[Indices.BaseColorIndex].Sample(texSampler, pin.TexCoord) * BaseColorFactor;
    if (baseColor.a < AlphaCutoff)
        discard;
    
    baseColor = pow(baseColor, 2.2f);
    
    float3 output = baseColor.rgb;
    
    float3 tangent = normalize(pin.Tangent - dot(pin.Tangent, pin.Normal) * pin.Normal);
    float3 bitangent = cross(pin.Normal, tangent);
    float3x3 texSpace = float3x3(tangent, bitangent, pin.Normal);
    
    float3 normalMap = normalize(2.0f * TexturesTable[Indices.NormalIndex].Sample(texSampler, pin.TexCoord).rgb - 1.0f);
    pin.Normal = normalize(mul(normalMap.xyz, texSpace));
    
    if (Indices.NormalIndex == -1)
    {
        output = baseColor.rgb * baseColor.rgb;
        output = output / (output + float3(1.0f, 1.0f, 1.0f));
        output = lerp(output, pow(output, 1.0f / 2.2f), 0.4f);
        return float4(output.rgb, 1.0f);
    }
    
    float metalness = 0.0f;
    float roughness = 0.0f;
    
    if (Indices.MetallicRoughnessIndex >= 0)
    {
        metalness = TexturesTable[Indices.MetallicRoughnessIndex].Sample(texSampler, pin.TexCoord).b * MetallicFactor;
        roughness = TexturesTable[Indices.MetallicRoughnessIndex].Sample(texSampler, pin.TexCoord).g * RoughnessFactor;
    }

    float3 N = pin.Normal;
    float3 V = normalize(CameraPosition - pin.WorldPosition.xyz);
    float3 reflection = reflect(-V, N);
    
    float3 cosLo = max(0.0f, dot(N, V));
    float3 Lr = 2.0f * cosLo * N - V;

    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, baseColor.xyz, metalness);
    
    // reflectance equation
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    // TEST
    for (int i = 0; i < LIGHTS; ++i)
    {
        float3 L = normalize(LightPositions[i].xyz - pin.WorldPosition.xyz);
        float3 H = normalize(V + L);
        
        float distance = length(LightPositions[i].xyz - pin.WorldPosition.xyz);
        float attenuation = 1.0f / (distance * distance);
        float3 radiance = LightColors[i].rgb * attenuation * 1000.0f;
        
        float NdotV = max(dot(N, V), 0.0f);
        float NdotH = max(dot(N, H), 0.0f);
        float NdotL = max(dot(N, L), Epsilon);
        
        // Cook-Torrance BRDF
        float NDF = GetDistributionGGX(N, H, roughness);
        float G = GetGeometrySmith(N, V, L, roughness);
        float3 F = GetFresnelSchlick(max(dot(H, V), 0.0f), F0);
        
        float3 kS = F;
        float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
        kD *= (1.0f - metalness);
    
        float3 numerator = NDF * G * F;
        float denominator = 4.0f * NdotV * NdotL + Epsilon;
        float3 specular = numerator / max(denominator, Epsilon);
        // * Ambient.rgb
        Lo += (kD * baseColor.rgb / PI + specular) * radiance * NdotL;
    }
   
    float3 ambient = float3(0.03f, 0.03f, 0.03f) * baseColor.rgb * float3(1.0f, 1.0f, 1.0f);
    output = baseColor.rgb * (ambient + Lo);

    if (Indices.EmissiveIndex >= 0)
    {
        float3 emissive = TexturesTable[Indices.EmissiveIndex].Sample(texSampler, pin.TexCoord).rgb * EmissiveFactor.rgb;
        output += emissive.rgb;
    }
    
    output *= baseColor.rgb;
    
    // Gamma correction
    output = output / (output + float3(1.0f, 1.0f, 1.0f));
    output = lerp(output, pow(output, 1.0f / 2.2f), 0.4f);

    return float4(output, 1.0f);
}

#endif // PBR_PIXEL_3_HLSL
