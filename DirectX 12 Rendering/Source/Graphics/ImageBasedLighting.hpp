#pragma once
#include "../Core/ComputePipelineState.hpp"
#include "Texture.hpp"
#include "TextureUtils.hpp"

#include "../Core/DescriptorHeap.hpp"

class DeviceContext;

class ImageBasedLighting
{
public:
	ImageBasedLighting() {}
	~ImageBasedLighting() {}

	void Create() {}

	void Test(DeviceContext* pDevice);
	void Test2(DeviceContext* pDevice, const std::string_view& Filepath);

	// Output texture from UAV
	ID3D12Resource* m_Texture{ nullptr };

	Descriptor m_UAVDesc;
	Descriptor m_SRVDesc;

	ComputePipelineState m_Pipeline;

private:



};
