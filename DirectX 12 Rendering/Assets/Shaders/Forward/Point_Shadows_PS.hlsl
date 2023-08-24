#ifndef POINT_SHADOWS_PS_HLSL
#define POINT_SHADOWS_PS_HLSL

#define LIGHTS_COUNT 4

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

cbuffer MatrixBuffer : register(b1, space0)
{
    row_major float4x4 LightViewProjection[4];
    float4 LightPosition[4];
    float4 LightColor[4];
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
Texture2D<float4> TexturesTable[] : register(t0, space1);
SamplerState texSampler : register(s0, space0);

Texture2D<float4> DepthTexture : register(t4, space0);
TextureCube SkyTexture : register(t5, space0);
TextureCube IrradianceTexture : register(t6, space0);
TextureCube SpecularTexture : register(t7, space0);
Texture2D<float4> SpecularBRDFTexture : register(t8, space0);

// https://github.com/matt77hias/RasterTek/tree/master/Tutorial%2040_Shadow%20Mapping/Engine
// https://rastertek.com/dx11tut40.html
// https://rastertek.com/dx11tut41.html
float4 main(PS_INPUT pin) : SV_TARGET
{
    float bias = 0.001f;
    
    float4 baseColor;
    if (Indices.BaseColorIndex >= 0)
    {
        baseColor = TexturesTable[Indices.BaseColorIndex].Sample(texSampler, pin.TexCoord) * BaseColorFactor;
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

    for (int i = 0; i < LIGHTS_COUNT; ++i)
    {
        
        float4 lightViewProjection = mul(LightViewProjection[i], float4(pin.WorldPosition.xyz, 1.0f));
        
        float2 projectTexCoord = float2(0.0f, 0.0f);
        projectTexCoord.x =  lightViewProjection.x / lightViewProjection.w / 2.0f + 0.5f;
        projectTexCoord.y = -lightViewProjection.y / lightViewProjection.w / 2.0f + 0.5f;

        float3 lightPos = normalize(LightPosition[i].xyz - pin.WorldPosition.xyz);
        
        if((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
        {
            // Sample the shadow map depth value from the depth texture using the sampler at the projected texture coordinate location.
            float depth = DepthTexture.Sample(texSampler, projectTexCoord).r;

		    // Calculate the depth of the light.
            
            float lightDepthValue = lightViewProjection.z / lightViewProjection.w;
            lightDepthValue = lightDepthValue - bias;

		    // Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
		    // If the light is in front of the object then light the pixel, if not then shadow this pixel since an object (occluder) is casting a shadow on it.
            if (lightDepthValue < depth)
            {
		    // Calculate the amount of light on this pixel.
                float lightIntensity = saturate(dot(pin.Normal, lightPos));
            //float lightIntensity = saturate(dot(pin.Normal, float3(-9.0f, 1.0f, 0.0f)));

                if (lightIntensity > 0.0f)
                {
				// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
                    output += LightColor[i].rgb * lightIntensity;
               // output += saturate((baseColor.rgb * lightIntensity));
                    output = saturate(output);
                }
            }
        }
    }

    output *= baseColor.rgb;

    return float4(output, 1.0f);
}

#endif // POINT_SHADOWS_PS_HLSL
