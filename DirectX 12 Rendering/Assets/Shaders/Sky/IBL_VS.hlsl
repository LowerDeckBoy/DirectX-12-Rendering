
cbuffer cbPerObject : register(b0)
{
    row_major float4x4 WVP;
    row_major float4x4 World;
    float4 padding[8];
};

struct VS_INPUT
{
    float3 Position : POSITION;
    // Unnecessary?
    float3 TexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 TexCoord : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT vin)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.Position = mul(WVP, float4(vin.Position, 1.0f)).xyzw;
    output.TexCoord = vin.Position;
    
    return output;
};