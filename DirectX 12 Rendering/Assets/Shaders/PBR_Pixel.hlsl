
cbuffer cbMaterial : register(b0)
{
    float4 Ambient;
    float3 Diffuse;
    float padding;
    float3 Specular;
    float SpecularIntensity;
    float3 Direction;
    float padding2;
    float Metallic;
    float Roughness;
    float2 padding3;
    float4 padding4[11];
};

//cbuffer cbCamera : register(b1)
//{
//    float3 CameraPosition;
//    float camPadding;
//};

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

static const float PI = 3.14159265359f;

// GGX/Trowbridge-Reitz
float GGX(float Alpha, float3 N, float3 H)
{
    float numerator = pow(Alpha, 2.0f);
    
    float NdotH = max(dot(N, H), 0.0f);
    
    float denominator = PI * pow(pow(NdotH, 2.0f) * (pow(Alpha, 2.0f) - 1.0f) + 1.0f, 2.0f);
    denominator = max(denominator, 0.000001f);

    return numerator / denominator;
}

// Schlik-Beckmann
// Shadowing function
float G1(float Alpha, float3 N, float3 X)
{
    float numerator = max(dot(N, X), 0.0f);
    float k = Alpha / 2.0f;
    
    float denominator = max(dot(N, X), 0.0f) * (1.0f - k) + k;
    denominator = max(denominator, 0.000001f);

    return numerator / denominator;
}

// Smith Model
float G(float Alpha, float3 N, float3 V, float3 L)
{
    return G1(Alpha, N, V) * G1(Alpha, N, L);
}

// Fresnel-Schlick 
float3 F(float3 F0, float3 V, float3 H)
{
    return F0 + (float3(1.0f, 1.0f, 1.0f) - F0) * pow(1 - max(dot(V, H), 0.0f), 5.0f);
}

Texture2D baseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metallicRoughnessTexture : register(t2);
Texture2D emissiveTexture : register(t3);

SamplerState baseSampler : register(s0);

//https://learnopengl.com/PBR/Lighting
//https://learnopengl.com/Advanced-Lighting/Deferred-Shading
float4 PS(PS_Input pin) : SV_TARGET
{
    float4 baseColor = baseTexture.Sample(baseSampler, pin.TexCoord) * Ambient;
    if (baseColor.a < 0.1f)
        discard;
    
    float3 normalMap = normalize(2.0f * normalTexture.Sample(baseSampler, pin.TexCoord).rgb - 1.0f);

    float metallicFactor  = metallicRoughnessTexture.Sample(baseSampler, pin.TexCoord).r;
    float roughnessFactor = metallicRoughnessTexture.Sample(baseSampler, pin.TexCoord).g;
    
    float3 tangent    = normalize(pin.Tangent - dot(pin.Tangent, pin.Normal) * pin.Normal);
    float3 bitangent  = cross(pin.Normal, tangent);
    float3x3 texSpace = float3x3(bitangent, tangent, pin.Normal);
    
    pin.Normal = normalize(mul(normalMap.xyz, texSpace));
    
    float3 N = pin.Normal;
    //float3 V = normalize(CameraPosition - pin.WorldPosition.xyz);
    float3 V = normalize(pin.ViewDirection - pin.WorldPosition.xyz);
    // Directional Light
    float3 L = normalize(Direction);
    
    float3 F0 = float3(0.04f, 0.04f, 0.04f);

    float3 Ks = F(F0, V, L);
    float3 Kd = float3(1.0f, 1.0f, 1.0f) - Ks;
    float3 lambert = baseColor.rgb / PI;
    
    float3 cookTorranceNum = GGX(roughnessFactor, N, L) * G(roughnessFactor, N, V, L) * F(F0, V, L);
    float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f);
    denominator = max(denominator, 0.000001f);
    float3 cookTorrance = cookTorranceNum / denominator;
    
    float3 BRDF = Kd * lambert + cookTorrance;
    float3 output = baseColor.rgb + (BRDF + Diffuse * max(dot(L, N), 0.0f));
    output *= baseColor.xyz;
    
    return float4(output, 1.0f);
}
