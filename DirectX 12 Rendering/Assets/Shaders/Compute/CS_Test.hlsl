
static const float PI = 3.14159265359f;
static const float2 invAtan = float2(0.1591f, 0.3183f);
// Converting equirectangular map (spherical) into cube format
// required if skybox is of cube shape instead of sphere
float2 EquirectangularToCube(float3 v)
{
    float2 uv = float2(atan2(v.z, v.x), -asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

Texture2D InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);
SamplerState texSampler	: register(s0);

[numthreads(64, 1, 1)]
void CS()
{
	//InputTexture.Sample(texSampler, )
	//float4 output = OutputTexture.Load()
	//float4(1.0f, 0.0, 1.0f, 1.0f);
}
