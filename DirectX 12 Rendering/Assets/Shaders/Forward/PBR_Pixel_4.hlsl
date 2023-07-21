#include "Lighting_PBR.hlsli"

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
    //float Metallic;
    //float Roughness;
    //float2 padding4;
    float4 padding5[11];
};

cbuffer cbLights : register(b1)
{
    float4 LightPositions[4];
    float4 LightColors[4];
    float lightsPadding[32];
}

Texture2D baseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metallicRoughnessTexture : register(t2);
Texture2D emissiveTexture : register(t3);

//Texture2D skyTexture : register(t4);
TextureCube skyTexture : register(t4);

SamplerState baseSampler : register(s0);

float4 main(PS_INPUT pin) : SV_TARGET
{
    float4 baseColor = baseTexture.Sample(baseSampler, pin.TexCoord);
    if (baseColor.a < 0.5f)
        discard;

    baseColor = pow(baseColor, 2.2f);

    float3 tangent = normalize(pin.Tangent - dot(pin.Tangent, pin.Normal) * pin.Normal);
    float3 bitangent = cross(pin.Normal, tangent);
    float3x3 texSpace = float3x3(tangent, bitangent, pin.Normal);

    float3 normalMap = normalize(2.0f * normalTexture.Sample(baseSampler, pin.TexCoord).rgb - 1.0f);
    pin.Normal = normalize(mul(normalMap.xyz, texSpace));

    float metalness = metallicRoughnessTexture.Sample(baseSampler, pin.TexCoord).b;
    float roughness = metallicRoughnessTexture.Sample(baseSampler, pin.TexCoord).g;

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
        float3 radiance = LightColors[i].rgb * attenuation * 50.0f;

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

    float3 emissive = emissiveTexture.Sample(baseSampler, pin.TexCoord).rgb * float3(1.0f, 1.0f, 1.0f);
    float3 ambient = float3(0.03f, 0.03f, 0.03f) * baseColor.rgb * float3(1.0f, 1.0f, 1.0f);
    float3 output = (ambient + Lo);

    if (any(emissive + float3(0.0f, 0.0f, 0.0f)))
    {
        output += emissive.rgb;
    }

    //float4 skyReflection = skyTexture.Sample(baseSampler, reflection.xy);
    float4 skyReflection = skyTexture.Sample(baseSampler, reflection.xyz);
    //float3 ambient = (kD * Diffuse * specular) * 1.0f;

    // + emissive.rgb

    // Gamma correction
    output = output / (output + float3(1.0f, 1.0f, 1.0f));
    output = lerp(output, pow(output, 1.0f / 2.2f), 0.4f);

    return float4(output, 1.0f);
}