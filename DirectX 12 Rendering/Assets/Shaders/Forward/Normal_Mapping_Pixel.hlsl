#ifndef NORMAL_MAPPING_PIXEL_HLSL
#define NORMAL_MAPPING_PIXEL_HLSL


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
};

struct PS_Input
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

ConstantBuffer<MaterialIndices> Indices : register(b1, space1);
Texture2D<float4> TexturesTable[] : register(t0, space1);
SamplerState texSampler : register(s0);

Texture2D<float4> SkyTexture : register(t4, space0);

float4 main(PS_Input pin) : SV_TARGET
{
    float4 baseColor = TexturesTable[Indices.BaseColorIndex].Sample(texSampler, pin.TexCoord);
    if (baseColor.a < AlphaCutoff)
        discard;
    
    baseColor = pow(baseColor, 2.2f);

    float3 normalMap = normalize(2.0f * TexturesTable[Indices.NormalIndex].Sample(texSampler, pin.TexCoord).rgb - 1.0f);
    
    float3 tangent = normalize(pin.Tangent - dot(pin.Tangent, pin.Normal) * pin.Normal);
    float3 bitangent = cross(pin.Normal, tangent);
    float3x3 texSpace = float3x3(bitangent, tangent, pin.Normal);
    
    pin.Normal = normalize(mul(normalMap.xyz, texSpace));
    
    //float3 output = baseColor.xyz + saturate(dot(pin.Normal, -Direction) * Diffuse.xyz * baseColor.xyz);
    float3 output = baseColor.xyz + saturate(dot(pin.Normal, -CameraPosition) * baseColor.xyz);
    output *= baseColor.xyz;
    
    if (Indices.EmissiveIndex >= 0)
    {
        float3 emissive = TexturesTable[Indices.EmissiveIndex].Sample(texSampler, pin.TexCoord).rgb;
        output += emissive;
    }

    output = output / (output + float3(1.0f, 1.0f, 1.0f));
    output = lerp(output, pow(output, 1.0f / 2.2f), 0.4f);

    return float4(output, 1.0f);
}

#endif // NORMAL_MAPPING_PIXEL_HLSL
