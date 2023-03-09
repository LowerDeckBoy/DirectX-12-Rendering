#pragma once
#include <d3d12.h>
#include <wrl.h>

#include <string>
#include <DirectXTex.h>

#include "../Core/DescriptorHeap.hpp"

class Device;


class Texture
{
public:
	Texture();
	Texture(Device* pDevice, const std::string& TexturePath);
	Texture operator=(const Texture& rhs);
	~Texture();

	void Create(Device* pDevice, const std::string& TexturePath);

	void Release();

	ID3D12Resource* GetTexture() const { return m_Texture.Get(); }
	const D3D12_GPU_VIRTUAL_ADDRESS& GetTextureGPUAddress() const { return m_Texture.Get()->GetGPUVirtualAddress(); }

	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;

	Descriptor m_Descriptor{};

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_cpuDescriptor{};
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_gpuDescriptor{};
	uint32_t m_DescriptorSize{ UINT32_MAX };

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

};