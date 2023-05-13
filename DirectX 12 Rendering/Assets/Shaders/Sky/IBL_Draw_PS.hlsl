
cbuffer cbMaterial : register(b0)
{
    float4 padding[16];
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 TexCoord : TEXCOORD;
};

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

Texture2D skyboxTexture : register(t0);
SamplerState texSampler : register(s0);
PS_INPUT pin;
//https://learnopengl.com/PBR/IBL/Diffuse-irradiance
//float4 PS(PS_INPUT pin) : SV_TARGET
float4 Prefilter(PS_INPUT pin)
{
    float2 uv = EquirectangularToCube(normalize(pin.TexCoord));
    float3 skyTexture = skyboxTexture.Sample(texSampler, uv).rgb;

    float3 irradiance = float3(0.0f, 0.0f, 0.0f);
    
    float3 normal = normalize(pin.Position.xyz);
    
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

            irradiance += skyboxTexture.Sample(texSampler, sampleVector.xy).rgb * cos(theta) * sin(theta);
            samplesCount++;
        }
    }
    
    irradiance = PI * irradiance * (1.0f / float(samplesCount));
    
    float3 gamma = float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f);
    skyTexture = skyTexture / (skyTexture + float3(1.0f, 1.0f, 1.0f));
    skyTexture = pow(skyTexture, gamma);
    //float3 radiance = skyboxTexture.Sample(texSampler, uv).rgb;
    //return float4(skyTexture, 1.0f);
    return float4(irradiance, 1.0f);
}

const static float4 filteredTexture = Prefilter(pin);


float4 main() : SV_TARGET
{
    float2 uv = EquirectangularToCube(normalize(pin.TexCoord));
    //float3 skyTexture = skyboxTexture.Sample(filteredTexture, uv).rgb;
    float3 skyTexture = filteredTexture.rgb;
    
    return float4(skyTexture, 1.0f);
}
