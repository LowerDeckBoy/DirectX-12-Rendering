#ifndef SPECULARMAP_HLSL
#define SPECULARMAP_HLSL

#include "Common_Compute.hlsli"

// Pre-filters environment cube map using GGX NDF importance sampling.
// Part of specular IBL split-sum approximation.

cbuffer SpecularMapFilterSettings : register(b0)
{
	// Roughness value to pre-filter for.
    float roughness;
};

TextureCube inputTexture : register(t0);
RWTexture2DArray<float4> outputTexture : register(u0);

SamplerState defaultSampler : register(s0);

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float Roughness)
{
    float alpha = Roughness * Roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

[numthreads(32, 32, 1)]
void main(uint3 ThreadID : SV_DispatchThreadID)
{
	// Make sure we won't write past output when computing higher mipmap levels.
    uint outputWidth  = 0;
    uint outputHeight = 0;
    uint outputDepth  = 0;
    outputTexture.GetDimensions(outputWidth, outputHeight, outputDepth);
    if (ThreadID.x >= outputWidth || ThreadID.y >= outputHeight)
        return;
 
	// Get input cubemap dimensions at zero mipmap level.
    float inputWidth, inputHeight, inputLevels;
    inputTexture.GetDimensions(0, inputWidth, inputHeight, inputLevels);

	// Solid angle associated with a single cubemap texel at zero mipmap level.
	// This will come in handy for importance sampling below.
    float wt = 4.0f * PI / (6.0f * inputWidth * inputHeight);
	
	// Approximation: Assume zero viewing angle (isotropic reflections).
    float3 N = GetSamplingVector(ThreadID, outputTexture);
    float3 Lo = N;
	
    float3 S = float3(0.0f, 0.0f, 0.0f);
    float3 T = float3(0.0f, 0.0f, 0.0f);
    ComputeBasisVectors(N, S, T);

    float3 color = float3(0.0f, 0.0f, 0.0f);
    float weight = 0.0f;

	// Convolve environment map using GGX NDF importance sampling.
	// Weight by cosine term since Epic claims it generally improves quality.
    for (uint i = 0; i < NumSamples; ++i)
    {
        float2 u = SampleHammersley(i);
        float3 Lh = TangentToWorld(SampleGGX(u.x, u.y, roughness), N, S, T);

		// Compute incident direction (Li) by reflecting viewing direction (Lo) around half-vector (Lh).
        float3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;

        float cosLi = dot(N, Li);
        if (cosLi > 0.0f)
        {
			// Use Mipmap Filtered Importance Sampling to improve convergence.
			// See: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html, section 20.4

            float cosLh = max(dot(N, Lh), 0.0f);

			// GGX normal distribution function (D term) probability density function.
			// Scaling by 1/4 is due to change of density in terms of Lh to Li (and since N=V, rest of the scaling factor cancels out).
            float pdf = ndfGGX(cosLh, roughness) * 0.25f;

			// Solid angle associated with this sample.
            float ws = 1.0f / (NumSamples * pdf);

			// Mip level to sample from.
            float mipLevel = max(0.5f * log2(ws / wt) + 1.0f, 0.0f);

            color += inputTexture.SampleLevel(defaultSampler, Li, mipLevel).rgb * cosLi;
            weight += cosLi;
        }
    }
    color /= weight;

    outputTexture[ThreadID] = float4(color, 1.0f);
}

#endif // SPECULARMAP_HLSL
