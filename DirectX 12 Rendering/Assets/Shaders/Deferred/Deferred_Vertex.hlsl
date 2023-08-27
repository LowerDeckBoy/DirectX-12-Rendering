#ifndef DEFERRED_VERTEX_HLSL
#define DEFERRED_VERTEX_HLSL

#include "Deferred_Lighting.hlsli"

ScreenQuadOutput main(ScreenQuadInput vin)
{
    ScreenQuadOutput output = (ScreenQuadOutput) 0;

    output.Position = float4(vin.Position.xyz, 1.0f);
    output.TexCoord = vin.TexCoord;

    return output;
}

#endif // DEFERRED_VERTEX_HLSL
