
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

    float4 padding4[10];
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

Texture2D baseTexture       : register(t0);
Texture2D normalTexture     : register(t1);
Texture2D metallicTexture   : register(t2);
Texture2D emissiveTexture   : register(t3);

SamplerState baseSampler : register(s0);

float4 main(PS_Input pin) : SV_TARGET
{
    float4 baseColor = baseTexture.Sample(baseSampler, pin.TexCoord);
    if (baseColor.a < AlphaCutoff)
        discard;
    
    baseColor = pow(baseColor, 2.2f);

    float3 normalMap = normalize(2.0f * normalTexture.Sample(baseSampler, pin.TexCoord).rgb - 1.0f);
    
    float3 tangent = normalize(pin.Tangent - dot(pin.Tangent, pin.Normal) * pin.Normal);
    float3 bitangent = cross(pin.Normal, tangent);
    float3x3 texSpace = float3x3(bitangent, tangent, pin.Normal);
    
    pin.Normal = normalize(mul(normalMap.xyz, texSpace));
    
    float3 output = baseColor.xyz + saturate(dot(pin.Normal, -Direction) * Diffuse.xyz * baseColor.xyz);
    output *= baseColor.xyz;
    
    if (bHasEmissive)
    {
        float3 emissive = emissiveTexture.Sample(baseSampler, pin.TexCoord).rgb;
        output += emissive;
    }

    output = output / (output + float3(1.0f, 1.0f, 1.0f));
    output = lerp(output, pow(output, 1.0f / 2.2f), 0.4f);

    return float4(output, 1.0f);
}
