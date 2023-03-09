#include "../Core/Device.hpp"
#include "Texture.hpp"
#include "../Utils/Utils.hpp"

//#include "../Core/Descriptors.hpp"
#include <D3D12MA/D3D12MemAlloc.h>


Texture::Texture()
{
	
}

Texture::Texture(Device* pDevice, const std::string& TexturePath)
{
	assert(m_Device = pDevice);
}

Texture Texture::operator=(const Texture& rhs)
{
	Texture result;
	result.m_Device = rhs.m_Device;
	result.m_Texture = rhs.m_Texture;
	result.m_TextureUploadHeap = rhs.m_TextureUploadHeap;
	result.m_Width = rhs.m_Width;
	result.m_Height = rhs.m_Height;
	result.m_Format = rhs.m_Format;
	result.m_Dimension = rhs.m_Dimension;
	return result;
}

Texture::~Texture()
{
	m_Device = nullptr;
}

void Texture::Create(Device* pDevice, const std::string& TexturePath)
{
	assert(m_Device = pDevice);

	std::wstring wpath = std::wstring(TexturePath.begin(), TexturePath.end());
	const wchar_t* path = wpath.c_str();

	DirectX::ScratchImage* scratchImage = new DirectX::ScratchImage();
	DirectX::LoadFromWICFile(path,
							 DirectX::WIC_FLAGS_FORCE_RGB,
							 nullptr,
							 *scratchImage);

	DirectX::TexMetadata metadata{ scratchImage->GetMetadata() };

	m_Width  = static_cast<uint32_t>(metadata.width);
	m_Height = static_cast<uint32_t>(metadata.height);
	m_Format = metadata.format;

	D3D12_RESOURCE_DESC desc{};
	desc.Format = metadata.format;
	desc.Width =  static_cast<uint32_t>(metadata.width);
	desc.Height = static_cast<uint32_t>(metadata.height);
	desc.MipLevels = 1;
	desc.DepthOrArraySize = metadata.depth;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE1D)
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE2D)
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;

	m_Dimension = desc.Dimension;

	// MA
	D3D12MA::ALLOCATION_DESC allocDesc{};
	allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
	D3D12MA::Allocation* allocation{};
	ThrowIfFailed(m_Device->GetAllocator()->CreateResource(&allocDesc,
				  &desc,
				  D3D12_RESOURCE_STATE_COPY_DEST,
				  nullptr,
				  &allocation,
				  IID_PPV_ARGS(m_Texture.GetAddressOf())));
	
	/*
	auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
	ThrowIfFailed(m_Device->GetDevice()->CreateCommittedResource(&heapProperties,
				  D3D12_HEAP_FLAG_NONE,
				  &desc,
				  D3D12_RESOURCE_STATE_COPY_DEST,
				  nullptr,
				  IID_PPV_ARGS(&m_Texture)));
	*/
	
	const uint64_t bufferSize{ GetRequiredIntermediateSize(m_Texture.Get(), 0, 1) };

	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	const auto bufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize) };
	ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(&heapProperties,
				  D3D12_HEAP_FLAG_NONE,
				  &bufferDesc,
				  D3D12_RESOURCE_STATE_GENERIC_READ,
				  nullptr,
				  IID_PPV_ARGS(m_TextureUploadHeap.GetAddressOf())));
	m_TextureUploadHeap->SetName(L"Texture Upload Heap");
	
	/*
	allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
	allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;
	
	ThrowIfFailed(m_Device->GetAllocator()->CreateResource(&allocDesc,
				  &desc,
				  D3D12_RESOURCE_STATE_GENERIC_READ,
				  nullptr,
				  &allocation,
				  IID_PPV_ARGS(m_TextureUploadHeap.GetAddressOf())));
	m_TextureUploadHeap->SetName(L"Texture Upload Heap");
	*/

	D3D12_SUBRESOURCE_DATA subresource{
		scratchImage->GetImages()->pixels,
		static_cast<LONG_PTR>(scratchImage->GetImages()->rowPitch),
		static_cast<LONG_PTR>(scratchImage->GetImages()->slicePitch)
	};
	UpdateSubresources(pDevice->GetCommandList(), m_Texture.Get(), m_TextureUploadHeap.Get(), 0, 0, 1, &subresource);

	const auto copyToResourceBarrier{ CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(),
																	 D3D12_RESOURCE_STATE_COPY_DEST,
																	 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) };
	m_Device->GetCommandList()->ResourceBarrier(1, &copyToResourceBarrier);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;


	m_Device->m_cbvDescriptorHeap.Allocate(m_Descriptor);

	m_Device->GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_Descriptor.m_cpuHandle);

	m_Texture->SetName(L"Texture SRV");
	allocation->Release();

	//https://asawicki.info/news_1754_direct3d_12_long_way_to_access_data
}

void Texture::Release()
{
}
