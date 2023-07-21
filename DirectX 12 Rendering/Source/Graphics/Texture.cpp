#include "../Core/DeviceContext.hpp"
#include "Texture.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Utilities/FileUtils.hpp"
//TEST
#include "../Core/ComputePipelineState.hpp"

#include <directxtk12/ResourceUploadBatch.h>
#include <directxtk12/DDSTextureLoader.h>
#include <directxtk12/WICTextureLoader.h>
#include "TextureUtils.hpp"


Texture::Texture()
{
	
}

Texture::Texture(DeviceContext* pDevice, const std::string& TexturePath)
{
	Create(pDevice, TexturePath);
}

Texture::Texture(DeviceContext* pDevice, const std::string& TexturePath, bool bSkyboxHDR)
{
	if (files::GetExtension(TexturePath) == ".dds")
	{
		CreateFromDDS(pDevice, TexturePath);
	}
	else if (files::GetExtension(TexturePath) == ".hdr")
	{
		CreateFromHDR(pDevice, TexturePath);
	}
	else
	{
		CreateFromWIC(pDevice, TexturePath);
	}
}

Texture::Texture(DeviceContext* pDevice, const std::string& TexturePath, const std::string& TextureName)
{
	CreateFromWIC(pDevice, TexturePath);
	SetName(TextureName);
}

Texture::~Texture()
{
	Release();
}

void Texture::Create(DeviceContext* pDevice, const std::string& TexturePath)
{
	if (files::GetExtension(TexturePath) == ".dds")
	{
		CreateFromDDS(pDevice, TexturePath);
	}
	else if (files::GetExtension(TexturePath) == ".hdr")
	{
		CreateFromHDR(pDevice, TexturePath);
	}
	else
	{
		CreateFromWIC(pDevice, TexturePath);
	}
}

void Texture::CreateFromWIC(DeviceContext* pDevice, const std::string& TexturePath)
{
	assert(m_Device = pDevice);

	std::wstring wpath = std::wstring(TexturePath.begin(), TexturePath.end());
	const wchar_t* path = wpath.c_str();

	DirectX::ScratchImage* scratchImage{ new DirectX::ScratchImage() };
	DirectX::LoadFromWICFile(path,
		DirectX::WIC_FLAGS_NONE,
		nullptr,
		*scratchImage);

	// https://github.com/microsoft/DirectXTK12/wiki/ResourceUploadBatch
	DirectX::ResourceUploadBatch upload(pDevice->GetDevice());
	upload.Begin();

	std::unique_ptr<uint8_t[]> decodedData;
	D3D12_SUBRESOURCE_DATA subresource{};

	DirectX::LoadWICTextureFromFileEx(pDevice->GetDevice(), path, 0, D3D12_RESOURCE_FLAG_NONE, DirectX::DX12::WIC_LOADER_MIP_AUTOGEN, m_Texture.ReleaseAndGetAddressOf(), decodedData, subresource);

	auto desc{ m_Texture->GetDesc() };

	auto uploadDesc = CD3DX12_RESOURCE_DESC(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		desc.Width, desc.Height, 1, desc.MipLevels,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		1, 0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE);

	ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(&HeapProps::HeapDefault,
		D3D12_HEAP_FLAG_NONE,
		&uploadDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(m_Texture.ReleaseAndGetAddressOf())));

	upload.Upload(m_Texture.Get(), 0, &subresource, 1);
	upload.Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	upload.GenerateMips(m_Texture.Get());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	m_Device->GetMainHeap()->Allocate(m_Descriptor);
	m_Device->GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_Descriptor.GetCPU());
	
	auto finish{ upload.End(m_Device->GetCommandQueue()) };
	finish.wait();
}

void Texture::CreateFromDDS(DeviceContext* pDevice, const std::string& TexturePath)
{
	assert(m_Device = pDevice);

	std::wstring wpath = std::wstring(TexturePath.begin(), TexturePath.end());
	const wchar_t* path = wpath.c_str();

	DirectX::ScratchImage* scratchImage{ new DirectX::ScratchImage() };
	DirectX::LoadFromDDSFile(path,
		DirectX::DDS_FLAGS_NONE,
		nullptr,
		*scratchImage);

	DirectX::TexMetadata metadata{ scratchImage->GetMetadata() };

	m_Width = static_cast<uint32_t>(metadata.width);
	m_Height = static_cast<uint32_t>(metadata.height);
	m_Format = metadata.format;

	D3D12_RESOURCE_DESC desc{};
	desc.Format = metadata.format;
	desc.Width = static_cast<uint32_t>(metadata.width);
	desc.Height = static_cast<uint32_t>(metadata.height);
	desc.MipLevels = 1;
	desc.DepthOrArraySize = metadata.IsCubemap() ? static_cast<uint16_t>(metadata.arraySize) : static_cast<uint16_t>(metadata.depth);
	desc.SampleDesc = { 1, 0 };

	if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE1D)
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE2D)
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;

	m_Dimension = desc.Dimension;

	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	for (uint8_t i = 0; i < 6; i++)
	{
		auto image{ scratchImage->GetImage(0, i, 0) };
		subresources.emplace_back(image->pixels, static_cast<LONG_PTR>(image->rowPitch), static_cast<LONG_PTR>(image->slicePitch));
	}

	ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(&HeapProps::HeapDefault,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(m_Texture.GetAddressOf())));

	const uint64_t bufferSize{ GetRequiredIntermediateSize(m_Texture.Get(), 0, static_cast<uint32_t>(subresources.size())) };

	const auto bufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize) };
	ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(&HeapProps::HeapUpload,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(m_TextureUploadHeap.GetAddressOf())));
	m_TextureUploadHeap->SetName(L"Texture Upload Heap");

	D3D12_SUBRESOURCE_DATA subresource{};
	subresource.pData = scratchImage->GetImages()->pixels;
	subresource.RowPitch = static_cast<LONG_PTR>(scratchImage->GetImages()->rowPitch);
	subresource.SlicePitch = static_cast<LONG_PTR>(scratchImage->GetImages()->slicePitch);

	UpdateSubresources(pDevice->GetCommandList(), m_Texture.Get(), m_TextureUploadHeap.Get(), 0, 0, static_cast<uint32_t>(subresources.size()), subresources.data());

	const auto copyToResourceBarrier{ CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) };
	m_Device->GetCommandList()->ResourceBarrier(1, &copyToResourceBarrier);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.MostDetailedMip = 0;

	m_Device->GetMainHeap()->Allocate(m_Descriptor);
	m_Device->GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_Descriptor.GetCPU());

	m_Texture->SetName(L"Texture SRV");
}

void Texture::CreateFromHDR(DeviceContext* pDevice, const std::string& TexturePath)
{
	assert(m_Device = pDevice);

	DirectX::ScratchImage* scratchImage{ new DirectX::ScratchImage() };
	//const wchar_t* path{ ToWchar(TexturePath) };
	std::wstring wpath = std::wstring(TexturePath.begin(), TexturePath.end());
	const wchar_t* path = wpath.c_str();

	DirectX::LoadFromHDRFile(path, nullptr, *scratchImage);

	DirectX::TexMetadata metadata{ scratchImage->GetMetadata() };

	D3D12_RESOURCE_DESC desc{};
	desc.Format = metadata.format;
	desc.Width = static_cast<uint32_t>(metadata.width);
	desc.Height = static_cast<uint32_t>(metadata.height);
	desc.MipLevels = 1;
	desc.DepthOrArraySize = metadata.IsCubemap() ? static_cast<uint16_t>(metadata.arraySize) : static_cast<uint16_t>(metadata.depth);
	desc.SampleDesc = { 1, 0 };
	desc.Flags = D3D12_RESOURCE_FLAG_NONE | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE1D)
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE2D)
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;

	ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(&HeapProps::HeapDefault,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(m_Texture.GetAddressOf())));

	const uint64_t bufferSize{ GetRequiredIntermediateSize(m_Texture.Get(), 0, 1) };

	const auto bufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize) };
	ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(&HeapProps::HeapUpload,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(m_TextureUploadHeap.GetAddressOf())));
	m_TextureUploadHeap->SetName(L"Texture Upload Heap");

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
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	m_Device->GetMainHeap()->Allocate(m_Descriptor);
	m_Device->GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_Descriptor.GetCPU());

	m_Texture->SetName(L"HDR Texture SRV");
}

void Texture::CreateTexture(DeviceContext* pDevice, uint32_t Width, uint32_t Height, DXGI_FORMAT Format)
{
	D3D12_RESOURCE_DESC desc{};
	desc.Width = Width;
	desc.Height = Height;
	desc.Format = Format;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.SampleDesc = { 1, 0 };
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(&HeapProps::HeapDefault, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_Texture)));
	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = desc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	pDevice->GetMainHeap()->Allocate(m_DescriptorSRV);
	pDevice->GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_DescriptorSRV.GetCPU());

}

void Texture::CreateUAV(ID3D12Resource* pTexture, uint32_t MipSlice)
{
	const auto desc{ pTexture->GetDesc() };
	assert(desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	m_Device->GetMainHeap()->Allocate(m_DescriptorUAV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = desc.Format;
	
	if (desc.DepthOrArraySize > 1)
	{
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Texture2DArray.MipSlice = MipSlice;
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		uavDesc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
	}
	else
	{
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = MipSlice;
	}

	m_Device->GetDevice()->CreateUnorderedAccessView(m_UavView.Get(), nullptr, &uavDesc, m_DescriptorUAV.GetCPU());
}

void Texture::Release()
{
	SAFE_RELEASE(m_UavView);
	SAFE_RELEASE(m_TextureUploadHeap);
	SAFE_RELEASE(m_Texture);
}
