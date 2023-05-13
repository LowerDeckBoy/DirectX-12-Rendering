#pragma once
#include <d3d12.h>
#include <wrl.h>

#include <string>
#include <DirectXTex.h>

#include "../Core/DescriptorHeap.hpp"

class Device;

// https://logins.github.io/graphics/2020/08/31/D3D12TexturesPart1.html

class Texture
{
public:
	Texture();
	Texture(Device* pDevice, const std::string& TexturePath);
	// bSkyboxHDR parameter indicates whether desired texture is meant for Skybox
	// and should be prefiltered for Image Based Lighting usage
	Texture(Device* pDevice, const std::string& TexturePath, bool bSkyboxHDR);
	Texture(Device* pDevice, const std::string& TexturePath, const std::string& TextureName);
	~Texture();

	// Determine texture loader based on input file extension
	void Create(Device* pDevice, const std::string& TexturePath);
	// No mipmapping variant
	//void Create(Device* pDevice, const std::string& TexturePath);
	// With mipmapping
	// via DirectXTK12
	void CreateFromWIC(Device* pDevice, const std::string& TexturePath);
	// Used for Skybox/Cubebox creating from DDS files
	void CreateFromDDS(Device* pDevice, const std::string& TexturePath);
	// HDR Skybox texture
	// by default without prefiltering
	void CreateFromHDR(Device* pDevice, const std::string& TexturePath);

	void CreateUAV(ID3D12Resource* pTexture, uint32_t MipSlice = 0);

	//void CreateFromHDR(Device* pDevice, const std::string& TexturePath, bool bPrefilter);

	void SetName(const std::string& NewName) { m_TextureName = NewName; }

	void Release();

	ID3D12Resource* GetTexture() const { return m_Texture.Get(); }
	const D3D12_GPU_VIRTUAL_ADDRESS GetTextureGPUAddress() const { return m_Texture.Get()->GetGPUVirtualAddress(); }
	std::string_view GetName() const { return m_TextureName.data(); }

	Descriptor m_Descriptor;
	Descriptor m_DescriptorUAV;

private:
	Device* m_Device{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> m_Texture;
	// Temporal solution
	// Making it local causes problems
	// with uploading to GPU before lifetime ends
	Microsoft::WRL::ComPtr<ID3D12Resource> m_TextureUploadHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_UavView;

	uint32_t m_Width{};
	uint32_t m_Height{};

	DXGI_FORMAT m_Format{ DXGI_FORMAT_R8G8B8A8_UNORM };
	D3D12_RESOURCE_DIMENSION m_Dimension{ D3D12_RESOURCE_DIMENSION_TEXTURE2D };

	std::string m_TextureName;
};