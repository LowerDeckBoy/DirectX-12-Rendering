
#define BLOCK_SIZE 8

#define GenerateMips_RootSignature \
    "RootFlags(0), " \
    "RootConstants(b0, num32BitConstants = 6), " \
    "DescriptorTable( SRV(t0, numDescriptors = 1) )," \
    "DescriptorTable( UAV(u0, numDescriptors = 4) )," \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)"

cbuffer cbMips : register(b0)
{
    uint SourceMipLevel;
    uint NumMipLevels;
    uint SourceDimensions;
    float2 TexelSize;
};

struct CS_INPUT
{
    uint3 GroupID           : SV_GroupID;
    uint3 GroupThreadID     : SV_GroupThreadID;
    uint3 DispatchThreadID  : SV_DispatchThreadID;
    uint  GroupIndex        : SV_GroupIndex;
};

Texture2D<float4> SourceMip : register(t0);

RWTexture2D<float4> OutMip_1 : register(u0);
RWTexture2D<float4> OutMip_2 : register(u1);
RWTexture2D<float4> OutMip_3 : register(u2);
RWTexture2D<float4> OutMip_4 : register(u3);

SamplerState baseSampler : register(s0);

groupshared float gs_R[64];
groupshared float gs_G[64];
groupshared float gs_B[64];
groupshared float gs_A[64];

void StoreColor(uint Index, float4 Color)
{
    gs_R[Index] = Color.r;
    gs_G[Index] = Color.g;
    gs_B[Index] = Color.b;
    gs_A[Index] = Color.a;
}

float4 LoadColor(uint Index)
{
    return float4(gs_R[Index], gs_G[Index], gs_B[Index], gs_A[Index]);

}

[RootSignature(GenerateMips_RootSignature)]
[numthreads( BLOCK_SIZE, BLOCK_SIZE, 1 )]
void main(CS_INPUT cin)
{
    float4 srcMip = (float4) 0;
    
    float2 uv = TexelSize * (cin.DispatchThreadID.xy + 0.5);
    
    srcMip = SourceMip.SampleLevel(baseSampler, uv, SourceMipLevel);
    
    OutMip_1 = srcMip;
    
    if (NumMipLevels == 1)
        return;
    
    StoreColor(cin.GroupIndex, srcMip);
    GroupMemoryBarrierWithGroupSync();
    
    float4 srcMip_02 = LoadColor(cin.GroupIndex + 0x01);
    float4 srcMip_03 = LoadColor(cin.GroupIndex + 0x08);
    float4 srcMip_04 = LoadColor(cin.GroupIndex + 0x09);

    srcMip = 0.25f * (srcMip + srcMip_02 + srcMip_03 + srcMip_04);
    
    OutMip_2[cin.DispatchThreadID.xy / 2] = srcMip;
    StoreColor(cin.GroupIndex, srcMip);

    if (NumMipLevels == 2)
        return;

}
