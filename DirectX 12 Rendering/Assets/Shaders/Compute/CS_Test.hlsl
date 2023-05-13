
//cbuffer cbCompute : register(b0)
//{
//    float3 UV : TEXCOORD;
//};

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

//Texture2D InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);
SamplerState texSampler	: register(s0);
/*
float4 Prefilter(float3 UV)
{
    float2 uv = EquirectangularToCube(normalize(UV));
    float3 skyTexture = InputTexture.Sample(texSampler, uv).rgb;

    float3 irradiance = float3(0.0f, 0.0f, 0.0f);
    
    float3 normal = normalize(UV);
    
    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));
    
    float sampleDelta = 0.025f;
    float samplesCount = 0.0f;
    for (float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta)
    {
        for (float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta)
        {
            // Spherical to Cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // Tangent to World
            float3 sampleVector = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            irradiance += InputTexture.Sample(texSampler, sampleVector.xy).rgb * cos(theta) * sin(theta);
            samplesCount++;
        }
    }
    
    irradiance = PI * irradiance * (1.0f / float(samplesCount));
    
    float3 gamma = float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f);
    skyTexture = skyTexture / (skyTexture + float3(1.0f, 1.0f, 1.0f));
    skyTexture = pow(skyTexture, gamma);

    return float4(irradiance, 1.0f);
}
*/

[numthreads(8, 8, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    OutputTexture[dispatchThreadID.xy] = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
	//InputTexture.Sample(texSampler, )
	//float4 output = OutputTexture.Load()
	//float4(1.0f, 0.0, 1.0f, 1.0f);
}
