
cbuffer cbMaterial : register(b0)
{
    float4 padding[16];
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 TexCoord : TEXCOORD;
};

static const float2 invAtan = float2(0.1591f, 0.3183f);
// Converting equirectangular map (spherical) into cube format
// required if skybox is of cube shape instead of sphere
float2 EquirectangularToCube(float3 v)
{
    float2 uv = float2(atan2(v.z, v.x), -asin(v.y));
    uv *= invAtan;
    uv += 0.5f;
    return uv;
}

//TextureCube skyboxTexture : register(t0);
Texture2D skyboxTexture : register(t0);
SamplerState texSampler : register(s0);

float4 PS(PS_INPUT pin) : SV_TARGET
{
    float2 uv = EquirectangularToCube(normalize(pin.TexCoord));
    float3 skyTexture = skyboxTexture.Sample(texSampler, uv).rgb;
    
    float3 gamma = float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f);
    skyTexture = skyTexture / (skyTexture + float3(1.0f, 1.0f, 1.0f));
    skyTexture = pow(skyTexture, gamma);
    
    return float4(skyTexture, 1.0f);
    //return float4(skyboxTexture.Sample(texSampler, pin.TexCoord.xy).rgb, 1.0f);
}