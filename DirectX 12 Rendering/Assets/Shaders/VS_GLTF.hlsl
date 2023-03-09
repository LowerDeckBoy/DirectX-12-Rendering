
//struct cbPerObject
//{
//    row_major float4x4 WVP;
//    row_major float4x4 World;
//    float4 padding[8];
//};

cbuffer cbPerObject : register(b0)
{
    row_major float4x4 WVP;
    row_major float4x4 World;
    float4 padding[8];
}

struct VS_Input
{
    float3 Position  : POSITION;
    float2 TexCoord  : TEXCOORD;
    float3 Normal    : NORMAL;
    float3 Tangent   : TANGENT;
    float3 Bitangent : BITANGENT;
};

struct VS_Output
{
    float4 Position     : SV_POSITION;
    float4 WorldPosition : WORLD_POSITION;
    float2 TexCoord     : TEXCOORD;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
    float3 Bitangent    : BITANGENT;
};

//ConstantBuffer<cbPerObject> cbData : register(b0);

VS_Output VS(VS_Input vin)
{
    VS_Output output = (VS_Output) 0;
    
    output.Position = mul(WVP, float4(vin.Position, 1.0f));
    output.WorldPosition = mul(World, float4(vin.Position, 1.0f));
    output.TexCoord  = vin.TexCoord; 
    output.Normal    = normalize(mul((float3x3)World, vin.Normal));
    output.Tangent   = normalize(mul((float3x3)World, vin.Tangent));
    output.Bitangent = normalize(mul((float3x3)World, vin.Bitangent));
    
	return output;
}