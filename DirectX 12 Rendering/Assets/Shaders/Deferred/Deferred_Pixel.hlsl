#ifndef DEFERRED_PIXEL_HLSL
#define DEFERRED_PIXEL_HLSL

#include "Deferred_Lighting.hlsli"

#define LIGHTS 4

cbuffer cbLights : register(b1, space1)
{
    float4 LightPositions[4];
    float4 LightColors[4];
    float Radius;
}

Texture2D<float4> BaseColorTexture      : register(t0, space2);
Texture2D<float4> NormalTexture         : register(t1, space2);
Texture2D<float4> MetalRoughnessTexture : register(t2, space2);
Texture2D<float4> EmissiveTexture       : register(t3, space2);
Texture2D<float4> PositionTexture       : register(t4, space2);
Texture2D<float4> DepthTexture          : register(t5, space2);
TextureCube SkyTexture                  : register(t6, space2);
TextureCube IrradianceTexture           : register(t7, space2);

SamplerState texSampler : register(s0);



float4 ClipToView(float4 clip)
{
    // View space position.
    //float4 view = mul(InversedProjection, clip);
    float4 view = mul(clip, Projection);

    // Perspective projection.
    view = view / view.w;

    return view;
}

// Convert screen space coordinates to view space.
float4 ScreenToView(float4 screen)
{
    // Convert to normalized texture coordinates
    float2 texCoord = screen.xy / ScreenDimension;
    // Convert to clip space
    float4 clip = float4(float2(texCoord.x, 1.0f - texCoord.y) * 2.0f - 1.0f, screen.z, screen.w);

    return ClipToView(clip);
}

[earlydepthstencil]
float4 main(ScreenQuadOutput pin) : SV_TARGET
{
    float2 position = pin.Position.xy;
    float z = DepthTexture.Load(float3(position.xy, 0)).x;
    
    // Get Position
    //float depth = Projection._43 / (z - Projection._33);
    //float4 posCS = float4(position * 2.0f - 1.0f, depth, 1.0f);
    //posCS.y *= -1.0f;
    //float4 positionWS = mul(posCS, InversedView * InversedProjection);

    float3 positions = PositionTexture.Load(int3(position, z)).rgb;
    float3 albedo = pow(BaseColorTexture.Load(int3(position, 0)).xyz, 2.2f);
    float4 normal = NormalTexture.Load(int3(position, 0));
    //normal = float4((normal.xyz - 0.5f) * 2.0f, normal.w);
    
    float3 metalRoughness = MetalRoughnessTexture.Load(int3(position, 0)).rgb;
    float3 emissive = EmissiveTexture.Load(int3(position, 0)).xyz;
    
    float3 N = normalize(normal.rgb);
    float3 V = normalize(CameraPosition.xyz - positions);
    
    float NdotV = saturate(max(dot(N, V), Epsilon));
    
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo.rgb, metalRoughness.b);
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    
    float3 reflection = normalize(reflect(-V, N));
    float3 sky = SkyTexture.Sample(texSampler, reflection.xyz).rgb * metalRoughness.b;

    for (int i = 0; i < LIGHTS; ++i)
    {
        float3 L = normalize(LightPositions[i].xyz - positions);
        float3 H = normalize(V + L);

        float distance = length(LightPositions[i].xyz - positions);
        float attenuation = 1.0f / (distance * distance);
        float3 radiance = LightColors[i].rgb * (attenuation * LightPositions[i].w);
        
        float NdotH = max(dot(N, H), 0.0f);
        float NdotL = max(dot(N, L), 0.0f);

        // Cook-Torrance BRDF
        float NDF = GetDistributionGGX(N, H, metalRoughness.g);
        float G = GetGeometrySmith(N, V, L, metalRoughness.g);
        float3 F = GetFresnelSchlick(max(dot(H, V), 0.0f), F0);

        float3 kS = F;
        float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - kS, float3(0.0f, 0.0f, 0.0f), metalRoughness.b);
        kD *= (1.0f - metalRoughness.b);
        
        float3 numerator = NDF * G * F;
        float denominator = 4.0f * NdotV * NdotL + Epsilon;
        float3 specular = numerator / max(denominator, Epsilon);
        // 
        Lo += (kD * albedo.rgb + sky / PI + specular) * radiance * NdotL;
    }

    float3 ambient = float3(0.03f, 0.03f, 0.03f) * albedo.rgb * float3(1.0f, 1.0f, 1.0f);
    float3 output = (ambient.rgb + Lo);

    float3 ambientLighting = float3(0.0f, 0.0f, 0.0f);
    {
        float3 irradiance = IrradianceTexture.Sample(texSampler, N).rgb;

        float3 F = GetFresnelSchlick(NdotV, F0);
        float3 kS = F;
        float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - kS, float3(0.0f, 0.0f, 0.0f), metalRoughness.b);
        
        float3 diffuseIBL = kD * albedo.rgb * irradiance;
        ambientLighting = diffuseIBL;
    }
   
    if (any(emissive))
    {
        output += emissive;
    }

    // Gamma correction
    output = output / (output + float3(1.0f, 1.0f, 1.0f));
    output = lerp(output, pow(output, 1.0f / 2.2f), 0.4f);

    return float4(output.rgb + ambientLighting, 1.0f);
}

#endif // DEFERRED_PIXEL_HLSL
