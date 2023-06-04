
#define LIGHTS 4

cbuffer cbMaterial : register(b0)
{
    float4 Ambient;
    float3 Diffuse;
    float padding;
    float3 Direction;
    float padding2;
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

// Light positions

static const float PI = 3.141592f;
static const float Epsilon = 0.00001f;

// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = 0.04f;

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : WORLD_POSITION;
    float3 ViewDirection : VIEW_DIRECTION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
};

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

Texture2D baseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metallicRoughnessTexture : register(t2);
Texture2D emissiveTexture : register(t3);

Texture2D SkyTexture : register(t4);

SamplerState baseSampler : register(s0);

float4 main(PS_INPUT pin) : SV_TARGET
{
    float3 output = float3(0.0f, 0.0f, 0.0f);

    float4 baseColor = baseTexture.Sample(baseSampler, pin.TexCoord) * BaseColorFactor;
    if (baseColor.a < AlphaCutoff)
        discard;

    baseColor = pow(baseColor, 2.2f);
    output = baseColor.rgb;

    float metalness = 0.0f;
    float roughness = 0.0f;

    if (bHasMetallic)
    {
        metalness = metallicRoughnessTexture.Sample(baseSampler, pin.TexCoord).b * MetallicFactor;
        roughness = metallicRoughnessTexture.Sample(baseSampler, pin.TexCoord).g * RoughnessFactor;
    }

    if (bHasNormal)
    {
    }
    float3 normalMap = normalize(2.0f * normalTexture.Sample(baseSampler, pin.TexCoord).rgb - 1.0f);
    float3 tangent = normalize(pin.Tangent - dot(pin.Tangent, pin.Normal) * pin.Normal);
    float3 bitangent = cross(pin.Normal, tangent);
    float3x3 texSpace = float3x3(tangent, bitangent, pin.Normal);
    pin.Normal = normalize(mul(normalMap.xyz, texSpace));

    float3 N = pin.Normal;
    float3 V = normalize(CameraPosition - pin.WorldPosition.xyz);

    float3 reflection = -normalize(reflect(V, N));

    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), baseColor.rgb, metalness);
    // reflectance equation
    //float3 sky = SkyTexture.SampleLevel(baseSampler, reflection.xz, 0).rgb;
    float3 sky = SkyTexture.SampleLevel(baseSampler, reflection.xy, 0).rgb;
    //float3 sky = SkyTexture.SampleLevel(baseSampler, N.xz, 0).rgb;
    // TEST
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < LIGHTS; ++i)
    {
        float3 L = normalize(LightPositions[i].xyz - pin.WorldPosition.xyz);
        float3 H = normalize(L + V);

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
        float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - kS, float3(.0f, 0.0f, 0.0f), metalness);
        //kD *= (1.0f - metalness);

        float3 numerator = NDF * G * F;
        float denominator = 4.0f * NdotV * NdotL + Epsilon;
        float3 specular = numerator / max(denominator, Epsilon);
        //
        Lo += (kD * baseColor.rgb / PI + specular) * radiance * NdotL;
    }

    float3 ambient = float3(0.03f, 0.03f, 0.03f) * baseColor.rgb * float3(1.0f, 1.0f, 1.0f);
    output = (ambient + Lo);
    // * baseColor.rgb
    if (metalness != 0.0f) {
        //output *= baseColor.rgb + saturate(sky);
        output += saturate(sky);
    }
    else {
        //output *= baseColor.rgb;
    }
    //output = lerp(output.rgb, output.rgb, baseColor.rgb);

    if (bHasEmissive)
    {
        float3 emissive = emissiveTexture.Sample(baseSampler, pin.TexCoord).rgb * EmissiveFactor.xyz;
        output += emissive;
    }

    
    output *= baseColor.rgb;
    //output *= baseColor.rgb;

    // Gamma correction
    output = output / (output + float3(1.0f, 1.0f, 1.0f));
    output = lerp(output, pow(output, 1.0f / 2.2f), 0.4f);

    return float4(output, 1.0f);
}