#ifndef EQUIRECTANGULAR2CUBE_HLSL
#define EQUIRECTANGULAR2CUBE_HLSL

static const float PI = 3.141592f;

Texture2D<float4> inputTexture : register(t0);
RWTexture2DArray<float4> outputTexture : register(u0);
SamplerState texSampler : register(s0);

float3 TransformCoords(uint3 ThreadID)
{
    float width, height, depth;
    outputTexture.GetDimensions(width, height, depth);

    float2 st = ThreadID.xy / float2(width, height);
    float2 uv = 2.0f * float2(st.x, 1.0f - st.y) - float2(1.0f, 1.0f);

	// Select vector based on cubemap face index.
    float3 output = float3(0.0f, 0.0f, 0.0f);
    switch (ThreadID.z)
    {
        case 0:
            output = float3(1.0f, uv.y, -uv.x);
            break;
        case 1:
            output = float3(-1.0f, uv.y, uv.x);
            break;
        case 2:
            output = float3(uv.x, 1.0f, -uv.y);
            break;
        case 3:
            output = float3(uv.x, -1.0f, uv.y);
            break;
        case 4:
            output = float3(uv.x, uv.y, 1.0f);
            break;
        case 5:
            output = float3(-uv.x, uv.y, -1.0f);
            break;
    }
    return normalize(output);
}

[numthreads(32, 32, 1)]
void main(uint3 ThreadID : SV_DispatchThreadID)
{
    float3 coords = TransformCoords(ThreadID);
	
	// Cartesian to spherical coords
    float phi = atan2(coords.z, coords.x);
    float theta = acos(coords.y);

	// Sample equirectangular texture.
    float4 output = inputTexture.SampleLevel(texSampler, float2(phi / (2.0f * PI), theta / PI), 0.0f);

    outputTexture[ThreadID] = output;
}

#endif // EQUIRECTANGULAR2CUBE_HLSL
