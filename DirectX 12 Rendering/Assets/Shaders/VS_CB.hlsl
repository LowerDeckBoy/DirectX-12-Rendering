//
cbuffer cbData : register(b0)
{
    float4x4 WVP;
    float4 padding[15];
    //float4 Offset;
    //float4 padding[15];
};

struct VS_Input
{
    float4 Position : POSITION;
    float2 UV : TexCoord;
};

struct VS_Output
{
    float4 Position : SV_POSITION;
    float2 UV : TexCoord;
};


VS_Output VS(VS_Input vin)
{
    VS_Output output = (VS_Output) 0;
    
    output.Position = mul(vin.Position, WVP);
    output.UV = vin.UV;
    
    return output;
}