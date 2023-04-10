
cbuffer cbMaterial : register(b0)
{
    float4 padding[16];
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 TexCoord : TEXCOORD;
};

TextureCube skyboxTexture : register(t0);
SamplerState texSampler : register(s0);

float4 PS(PS_INPUT pin) : SV_TARGET
{
    return skyboxTexture.Sample(texSampler, pin.TexCoord);
}