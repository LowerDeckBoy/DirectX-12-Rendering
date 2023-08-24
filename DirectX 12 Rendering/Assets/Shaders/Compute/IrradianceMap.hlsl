#ifndef IRRADIANCEMAP_HLSL
#define IRRADIANCEMAP_HLSL

#include "Common_Compute.hlsli"

TextureCube inputTexture : register(t0);
RWTexture2DArray<float4> outputTexture : register(u0);
SamplerState texSampler : register(s0);

[numthreads(32, 32, 1)]
void main(uint3 ThreadID : SV_DispatchThreadID)
{
    float3 N = GetSamplingVector(ThreadID, outputTexture);
	
    float3 S, T;
    ComputeBasisVectors(N, S, T);

	// Monte Carlo integration of hemispherical irradiance.
	// As a small optimization this also includes Lambertian BRDF assuming perfectly white surface (albedo of 1.0)
	// so we don't need to normalize in PBR fragment shader (so technically it encodes exitant radiance rather than irradiance).
    float3 irradiance = 0.0f;
    for (uint i = 0; i < NumSamples; ++i)
    {
        float2 u = SampleHammersley(i);
        float3 Li = TangentToWorld(SampleHemisphere(u.x, u.y), N, S, T);
        float cosTheta = max(0.0, dot(Li, N));

		// PIs here cancel out because of division by pdf.
        irradiance += 2.0 * inputTexture.SampleLevel(texSampler, Li, 0).rgb * cosTheta;
    }
    irradiance /= float(NumSamples);

    outputTexture[ThreadID] = float4(irradiance, 1.0);
}

#endif // IRRADIANCEMAP_HLSL
