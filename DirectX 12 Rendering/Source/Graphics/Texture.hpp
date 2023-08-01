#pragma once
#include <d3d12.h>
#include <string_view>
#include "../Core/DescriptorHeap.hpp"

class DeviceContext;

class Texture
{
public:
	Texture();
	Texture(DeviceContext* pDevice, const std::string_view& TexturePath);
	~Texture();

	// Determine texture loader based on input file extension
	void Create(DeviceContext* pDevice, const std::string_view& TexturePath);
	// No mipmapping variant
	//void Create(DeviceContext* pDevice, const std::string& TexturePath);
	// With mipmapping via DirectXTK12
	void CreateFromWIC(DeviceContext* pDevice, const std::string_view& TexturePath);
	// Used for Skybox/Cubebox creating from DDS files
	void CreateFromDDS(DeviceContext* pDevice, const std::string_view& TexturePath);
	// HDR Skybox texture
	// by default without prefiltering -> non IBL
	// and Equirectangular
	void CreateFromHDR(DeviceContext* pDevice, const std::string_view& TexturePath);

	void Release();

	inline ID3D12Resource* GetTexture() const noexcept { return m_Texture.Get(); }
	const D3D12_GPU_VIRTUAL_ADDRESS GetTextureGPUAddress() const { return m_Texture.Get()->GetGPUVirtualAddress(); }

	Descriptor m_Descriptor;
	Descriptor m_DescriptorUAV;
	Descriptor m_DescriptorSRV;

private:
	//DeviceContext* m_Device{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> m_Texture;
	// Temporal solution
	// Making it local causes problems
	// with uploading to GPU before lifetime ends
	Microsoft::WRL::ComPtr<ID3D12Resource> m_TextureUploadHeap;

	// Hardcoded amount of mipmaps in order to reduce memory usage
	uint16_t m_MipLevels{ 5 };
	uint32_t m_Width{};
	uint32_t m_Height{};

	DXGI_FORMAT m_Format{ DXGI_FORMAT_R8G8B8A8_UNORM };
	D3D12_RESOURCE_DIMENSION m_Dimension{ D3D12_RESOURCE_DIMENSION_TEXTURE2D };
};
