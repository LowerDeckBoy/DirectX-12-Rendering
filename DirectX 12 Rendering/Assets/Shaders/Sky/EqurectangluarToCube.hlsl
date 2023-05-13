
Texture2D<float4>   InputTexture	: register(t0);
RWTexture2D<float4> OutputTexture	: register(u0);
SamplerState		texSampler		: register(s0);

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

[numthreads(8, 8, 1)]
//float3 UV, uint3 dispatchThreadID : SV_DispatchThreadID
void main(uint3 ThreadID : SV_DispatchThreadID)
{
	//return float2(1.0f, 1.0f);
	//return EquirectangularToCube(UV);
}