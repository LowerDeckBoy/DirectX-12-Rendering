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
	Texture(Device* pDevice, const std::string& TexturePath, const std::string& TextureName);
	~Texture();

	// TODO: Add mipmapping 
	void Create(Device* pDevice, const std::string& TexturePath);
	void SetName(const std::string& NewName) { m_TextureName = NewName; }

	void Release();

	ID3D12Resource* GetTexture() const { return m_Texture.Get(); }
	const D3D12_GPU_VIRTUAL_ADDRESS GetTextureGPUAddress() const { return m_Texture.Get()->GetGPUVirtualAddress(); }

	Descriptor m_Descriptor;

private:
	Device* m_Device;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_Texture;
	// Temporal solution
	// Making it local causes problems
	// with uploading to GPU before lifetime ends
	Microsoft::WRL::ComPtr<ID3D12Resource> m_TextureUploadHeap;

	uint32_t m_Width{};
	uint32_t m_Height{};
	DXGI_FORMAT m_Format{ DXGI_FORMAT_R8G8B8A8_UNORM };
	D3D12_RESOURCE_DIMENSION m_Dimension{ D3D12_RESOURCE_DIMENSION_TEXTURE2D };
	std::string m_TextureName;
};