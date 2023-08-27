#ifndef PBR_PIXEL_TEST_HLSL
#define PBR_PIXEL_TEST_HLSL

#include "Lighting_PBR.hlsli"

#define LIGHTS 4

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
}

ConstantBuffer<MaterialIndices> Indices : register(b0, space2);

Texture2D<float4> TexturesTable[]       : register(t0, space1);
SamplerState texSampler                 : register(s0, space0);

//Texture2D<float4> DepthTexture : register(t4, space0);
TextureCube SkyTexture                  : register(t5, space0);
TextureCube IrradianceTexture           : register(t6, space0);
TextureCube SpecularTexture             : register(t7, space0);
Texture2D<float4> SpecularBRDFTexture   : register(t8, space0);

float4 main(PS_INPUT pin) : SV_TARGET
{
    float4 baseColor = BaseColorFactor;
    if (Indices.BaseColorIndex >= 0)
    {
        float4 diffuse = TexturesTable[Indices.BaseColorIndex].Sample(texSampler, pin.TexCoord);
        if (diffuse.a < AlphaCutoff)
            discard;
        
        clip(diffuse.a - 0.1f);
        baseColor = pow(diffuse * BaseColorFactor, 2.2f);
    }

    float metalness = 0.0f;
    float roughness = 0.0f;

    if (Indices.MetallicRoughnessIndex >= 0)
    {
        metalness = TexturesTable[Indices.MetallicRoughnessIndex].Sample(texSampler, pin.TexCoord).b * MetallicFactor;
        roughness = TexturesTable[Indices.MetallicRoughnessIndex].Sample(texSampler, pin.TexCoord).g * RoughnessFactor;
    }

    if (Indices.NormalIndex >= 0)
    {
        float3 normalMap = normalize(2.0f * TexturesTable[Indices.NormalIndex].Sample(texSampler, pin.TexCoord).rgb - 1.0f);
        float3 tangent = normalize(pin.Tangent - dot(pin.Tangent, pin.Normal) * pin.Normal);
        float3 bitangent = cross(pin.Normal, tangent);
        float3x3 texSpace = float3x3(tangent, bitangent, pin.Normal);
        
        pin.Normal = normalize(mul(normalMap.xyz, texSpace));
    }
    
    float3 N = pin.Normal;
    float3 V = normalize(CameraPosition - pin.WorldPosition.xyz);
    
    float NdotV = saturate(max(dot(N, V), Epsilon));

    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), baseColor.rgb, metalness);

    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    
    float3 reflection = reflect(-V, N);
    
    for (int i = 0; i < LIGHTS; ++i)
    {
        float3 L = normalize(LightPositions[i].xyz - pin.WorldPosition.xyz);
        float3 H = normalize(V + L);

        float distance = length(LightPositions[i].xyz - pin.WorldPosition.xyz);
        float attenuation = 1.0f / (distance * distance);
        float3 radiance = LightColors[i].rgb * (attenuation * LightPositions[i].w);
        
        float NdotH = max(dot(N, H), 0.0f);
        float NdotL = max(dot(N, L), 0.0f);

        // Cook-Torrance BRDF
        float NDF = GetDistributionGGX(N, H, roughness);
        float G = GetGeometrySmith(N, V, L, roughness);
        float3 F = GetFresnelSchlick(max(dot(H, V), 0.0f), F0);

        float3 kS = F;
        float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - kS, float3(0.0f, 0.0f, 0.0f), metalness);
        kD *= (1.0f - metalness);
        
        float3 numerator = NDF * G * F;
        float denominator = 4.0f * NdotV * NdotL + Epsilon;
        float3 specular = numerator / max(denominator, Epsilon);

        Lo += (kD * baseColor.rgb / PI + specular) * radiance * NdotL;
    }
 
    float3 ambient = float3(0.03f, 0.03f, 0.03f) * baseColor.rgb * float3(1.0f, 1.0f, 1.0f);
    float3 output = (ambient + Lo);
 
    //if (Indices.NormalIndex == -1)
    //{
    //    output = baseColor.rgb + Lo;
    //    output = output / (output + float3(1.0f, 1.0f, 1.0f));
    //    output = lerp(output, pow(output, 1.0f / 2.2f), 0.4f);
    //    return float4(output.rgb, 1.0f);
    //}
    
    float3 ambientLighting = float3(0.0f, 0.0f, 0.0f);
    {
        float3 F = GetFresnelSchlick(NdotV, F0);
        float3 kS = F;
        float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - kS, float3(0.0f, 0.0f, 0.0f), metalness);
        kD *= (1.0f - metalness);
        
        float3 irradiance = IrradianceTexture.Sample(texSampler, N).rgb;
        float3 diffuseIBL = kD * irradiance;
        
        float3 specular = SpecularTexture.SampleLevel(texSampler, reflection, roughness).rgb;
        float2 specularBRDF = SpecularBRDFTexture.Sample(texSampler, float2(NdotV, roughness)).rg;
        float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specular;
        
        ambientLighting = (diffuseIBL + specularIBL) * metalness;
    }
    
    if (Indices.EmissiveIndex >= 0)
    {
        float3 emissive = TexturesTable[Indices.EmissiveIndex].Sample(texSampler, pin.TexCoord).rgb * EmissiveFactor.xyz;
        output += emissive;
    }
    
    // Gamma correction
    output = output / (output + float3(1.0f, 1.0f, 1.0f));
    output = lerp(output, pow(output, 1.0f / 2.2f), 0.4f);

    return float4(output + ambientLighting, 1.0f);
}

#endif // PBR_PIXEL_TEST_HLSL
