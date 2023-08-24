#ifndef LIGHTING_PBR_HLSLI
#define LIGHTING_PBR_HLSLI

// Helper file to share functions across pixel shaders

// Data for common PBR Shader usage
struct PS_INPUT
{
    float4 Position         : SV_POSITION;
    float4 WorldPosition    : WORLD_POSITION;
    float3 ViewDirection    : VIEW_DIRECTION;
    float2 TexCoord         : TEXCOORD;
    float3 Normal           : NORMAL;
    float3 Tangent          : TANGENT;
    float3 Bitangent        : BITANGENT;
};

struct MaterialIndices
{
    int BaseColorIndex;
    int NormalIndex;
    int MetallicRoughnessIndex;
    int EmissiveIndex;
};

static const float PI = 3.141592f;
static const float TwoPI = PI * 2.0f;
static const float Epsilon = 0.00001f;

// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = 0.04f;

float3 GetFresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float3 GetFresnelSchlickRoughness(float cosTheta, float3 F0, float Roughness)
{
    return F0 + (max(float3(1.0f - Roughness, 1.0f - Roughness, 1.0f - Roughness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
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
float ndfGGX(float cosLh, float roughness)
{
    float alpha = roughness * roughness;
    float sqAlpha = alpha * alpha;

    float denom = (cosLh * cosLh) * (sqAlpha - 1.0f) + 1.0f;
    return sqAlpha / (PI * denom * denom);
}

#endif //LIGHTING_PBR_HLSLI