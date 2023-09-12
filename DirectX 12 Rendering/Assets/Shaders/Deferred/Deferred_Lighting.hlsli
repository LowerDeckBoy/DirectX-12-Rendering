#ifndef DEFERRED_PBR_HLSLI
#define DEFERRED_PBR_HLSLI

// Helper for PBR rendering functions

// Helper structs
cbuffer cbPerObject : register(b0, space0)
{
    row_major float4x4 WVP;
    row_major float4x4 World;
}

cbuffer cbCamera : register(b1, space0)
{
    float4 CameraPosition;
    float4x4 View;
    float4x4 Projection;
    row_major float4x4 InversedView;
    row_major float4x4 InversedProjection;
    float2 ScreenDimension;
}

struct ScreenQuadInput
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct ScreenQuadOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

struct MaterialData
{
    float4 Albedo;
    float4 Normal;
    float4 MetalRoughness;
    float4 Emissive;
    float4 Positions;
};

// Helper file to share functions across pixel shaders
static const float PI = 3.141592f;
static const float TwoPI = PI * 2.0f;
static const float Epsilon = 0.00001f;

// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = 0.04f;

float3 GetFresnelSchlick(float NdotV, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - NdotV, 5.0f);
}

float3 GetFresnelSchlickRoughness(float NdotV, float3 F0, float Roughness)
{
    return F0 + (max(float3(1.0f - Roughness, 1.0f - Roughness, 1.0f - Roughness), F0) - F0) * pow(1.0f - NdotV, 5.0f);
}

// Normal Distribution
float GetDistributionGGX(float3 Normal, float3 H, float Roughness)
{
    float alpha = Roughness * Roughness;
    float alphaSq = alpha * alpha;
    float NdotH = max(dot(Normal, H), 0.0f);
    float NdotHsq = NdotH * NdotH;

    // Denormalize
    float denom = (NdotHsq * (alphaSq - 1.0f) + 1.0f);
    denom = PI * denom * denom;

    return alphaSq / denom;
}

float GetGeometrySchlickGGX(float NdotV, float Roughness)
{
    float r = (Roughness + 1.0f);
    float k = (r * r) / 8.0f;

    float denom = NdotV * (1.0f - k) + k;

    return NdotV / denom;
}

float GetGeometrySmith(float3 Normal, float3 V, float3 L, float Roughness)
{
    float NdotV = max(dot(Normal, V), 0.0f);
    float NdotL = max(dot(Normal, L), 0.0f);

    float ggx1 = GetGeometrySchlickGGX(NdotL, Roughness);
    float ggx2 = GetGeometrySchlickGGX(NdotV, Roughness);

    return ggx1 * ggx2;
}

//normal distribution
float ndfGGX(float NdotV, float Roughness)
{
    float alpha = Roughness * Roughness;
    float sqrtAlpha = alpha * alpha;

    float denom = (NdotV * NdotV) * (sqrtAlpha - 1.0f) + 1.0f;
    return sqrtAlpha / (PI * denom * denom);
}

// Lo output
float3 CalculateLighting(uint LightCount, in MaterialData Material, float3 V, in float4 LightPositions[4], in float4 LightColors[4])
{
    float NdotV = max(dot(Material.Normal.rgb, V), 0.0f);
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), Material.Albedo.rgb, Material.MetalRoughness.b);
    
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < LightCount; ++i)
    {
        float3 L = normalize(LightPositions[i].xyz - Material.Positions.xyz);
        float3 H = normalize(V + L);

        float distance = length(LightPositions[i].xyz - Material.Positions.xyz);
        float attenuation = 1.0f / (distance * distance);
        float3 radiance = LightColors[i].rgb * (attenuation * LightPositions[i].w);
        
        float NdotH = max(dot(Material.Normal.rgb, H), 0.0f);
        float NdotL = max(dot(Material.Normal.rgb, L), 0.0f);
        ;
        
        // Cook-Torrance BRDF
        float NDF = GetDistributionGGX(Material.Normal.rgb, H, Material.MetalRoughness.g);
        float G = GetGeometrySmith(Material.Normal.rgb, V, L, Material.MetalRoughness.g);
        float3 F = GetFresnelSchlick(max(dot(H, V), 0.0f), F0);

        float3 kS = F;
        float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - kS, float3(0.0f, 0.0f, 0.0f), Material.MetalRoughness.b);
        kD *= ((1.0f - Material.MetalRoughness.b) * Material.Normal.a);
        
        float3 numerator = NDF * G * F;
        float denominator = 4.0f * NdotV * NdotL;
        float3 specular = numerator / max(denominator, Epsilon);
        
        Lo += (kD * Material.Albedo.rgb / PI + specular) * radiance * NdotL;
    }
    
    return Lo;
}


#endif //DEFERRED_PBR_HLSLI
