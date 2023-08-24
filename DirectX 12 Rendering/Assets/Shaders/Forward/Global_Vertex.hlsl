#ifndef GLOBAL_VERTEX_HLSL
#define GLOBAL_VERTEX_HLSL

#include "Common.hlsli"


VS_Output main(VS_Input vin)
{
    VS_Output output = (VS_Output) 0;
    
    output.Position         = mul(WVP, float4(vin.Position, 1.0f));
    output.WorldPosition    = mul(World, float4(vin.Position, 1.0f));
    output.TexCoord         = vin.TexCoord; 
    output.Normal           = normalize(mul((float3x3)World, vin.Normal));
    output.Tangent          = normalize(mul((float3x3)World, vin.Tangent));
    output.Bitangent        = normalize(mul((float3x3)World, vin.Bitangent));
    
    output.ViewDirection    = normalize(CameraPosition.xyz - output.WorldPosition.xyz);
    
	return output;
}

#endif // GLOBAL_VERTEX_HLSL
