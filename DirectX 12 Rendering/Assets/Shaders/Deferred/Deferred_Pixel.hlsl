#ifndef DEFERRED_PIXEL_HLSL
#define DEFERRED_PIXEL_HLSL

#include "Deferred_Lighting.hlsli"

#define LIGHTS 4

cbuffer cbLights : register(b1, space1)
{
    float4 LightPositions[4];
    float4 LightColors[4];
    float Radius;
    float lightsPadding[32];
}

Texture2D<float4> BaseColorTexture      : register(t0, space2);
Texture2D<float4> NormalTexture         : register(t1, space2);
Texture2D<float4> MetalRoughnessTexture : register(t2, space2);
Texture2D<float4> EmissiveTexture       : register(t3, space2);
Texture2D<float4> PositionTexture       : register(t4, space2);
Texture2D<float4> DepthTexture          : register(t5, space2);
Texture2D<float4> SkyTexture            : register(t6, space2);

SamplerState texSampler : register(s0);

[earlydepthstencil]
float4 main(ScreenQuadOutput pin) : SV_TARGET
{
    float2 position = pin.Position.xy;
    float2 dims = position / ScreenDimension;
    float z = DepthTexture.Load(float3(position.xy, 0)).x;

    // Get Position
    float depth = Projection._43 / (z - Projection._33);
    float4 posCS = float4(position * 2.0f - 1.0f, depth, 1.0f);
    //float4 posCS = float4(dims * 2.0f - 1.0f, depth, 1.0f);
    posCS.y *= -1.0f;
    float4 positionWS = mul(posCS, InversedView * InversedProjection);
    float3 outPosition = positionWS.rgb / positionWS.w;
    
    float3 positions = PositionTexture.Load(int3(outPosition.xyz)).rgb;
    float3 albedo = pow(BaseColorTexture.Load(int3(position, 0)).xyz, 2.2f);
    float4 normal = normalize(NormalTexture.Load(int3(position, 0)));
    normal = float4((normal.xyz - 0.5f) * 2.0f, normal.w);
    
    float metalness = MetalRoughnessTexture.Load(int3(position, 0)).b;
    float roughness = MetalRoughnessTexture.Load(int3(position, 0)).g;
    float3 emissive = EmissiveTexture.Load(int3(position, 0)).xyz;
    
    float3 N = normal.rgb;
    float3 V = normalize(CameraPosition.xyz - positions);
    
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo.rgb, metalness);
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    
    
    for (int i = 0; i < LIGHTS; ++i)
    {
        float3 L = -(normalize((-LightPositions[i].xyz - positions)));
        float3 H = normalize(L + V);

        float distance = length(L);
        
        //if (distance < 25.0f)
        //{
        //    continue;
        //}
        
        float attenuation = 1.0f / (distance * distance);
        float3 radiance = LightColors[i].rgb * attenuation * 25.0f;

        float NdotV = saturate(max(dot(N, V), Epsilon));
        float NdotH = saturate(max(dot(N, H), Epsilon));
        float NdotL = max(dot(N, L), Epsilon);

        // Cook-Torrance BRDF
        float NDF = GetDistributionGGX(N, H, roughness);
        float G = GetGeometrySmith(N, V, L, roughness);
        float3 F = GetFresnelSchlick(max(dot(H, V), 0.0f), F0);

        float3 kS = F;
        float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - kS, float3(0.0f, 0.0f, 0.0f), metalness);

        float3 numerator = NDF * G * F;
        float denominator = 4.0f * NdotV * NdotL + Epsilon;
        float3 specular = numerator / max(denominator, Epsilon);

        float3 reflection = reflect(-LightPositions[i].xyz, N);
        float3 sky = SkyTexture.Load(int3(reflection.xyz)).rgb;
        
        Lo += (kD * albedo.rgb / PI + specular) * radiance * NdotL + sky;
    }
    
    // 
    
    float3 ambient = float3(0.03f, 0.03f, 0.03f) * albedo.rgb * float3(1.0f, 1.0f, 1.0f);
    float3 output = (ambient.rgb + saturate(Lo)) * albedo.rgb;
    
    float3 reflection = reflect(LightPositions[0].xyz, N);
    float3 sky = SkyTexture.Load(int3(reflection.xyz)).rgb;
    //float3 sky = saturate(SkyTexture.Sample(texSampler, int2(reflection.xy)).rgb);
    //output += sky;

    if (any(emissive))
    {
        output += emissive;
    }
    
    // Gamma correction
    output = output / (output + float3(1.0f, 1.0f, 1.0f));
    output = lerp(output, pow(output, 1.0f / 2.2f), 0.4f);

    return float4(output.rgb, 1.0f);
}

#endif // DEFERRED_PIXEL_HLSL
