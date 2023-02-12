
struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 UV : TexCoord;
};

Texture2D tex : register(t0);
SamplerState samplerState : register(s0);

float4 PS(PS_Input pin) : SV_TARGET
{
    return tex.Sample(samplerState, pin.UV);
}