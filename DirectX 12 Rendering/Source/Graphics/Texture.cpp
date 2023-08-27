#include "../Core/DeviceContext.hpp"
#include "Texture.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Utilities/FileUtils.hpp"
#include <DirectXTex.h>
#include <directxtk12/ResourceUploadBatch.h>
//#include <directxtk12/DDSTextureLoader.h>
#include <directxtk12/WICTextureLoader.h>
#include "TextureUtils.hpp"


Texture::Texture()
{
	
}

Texture::Texture(DeviceContext* pDevice, const std::string_view& TexturePath)
{
	Create(pDevice, TexturePath);
}

Texture::~Texture()
{
	Release();
}

void Texture::Create(DeviceContext* pDevice, const std::string_view& TexturePath)
{
	if (files::GetExtension(TexturePath.data()) == ".dds")
	{
		CreateFromDDS(pDevice, TexturePath);
	}
	else if (files::GetExtension(TexturePath.data()) == ".hdr")
	{
		CreateFromHDR(pDevice, TexturePath);
	}
	else
	{
		CreateFromWIC(pDevice, TexturePath);
		//m_Texture = TextureUtils::CreateFromWIC(pDevice, m_Descriptor, TexturePath);
	}
}

void Texture::CreateFromWIC(DeviceContext* pDevice, const std::string_view& TexturePath)
{
	//assert(m_Device = pDevice);

	std::wstring wpath = std::wstring(TexturePath.begin(), TexturePath.end());
	const wchar_t* path = wpath.c_str();

	DirectX::ScratchImage* scratchImage{ new DirectX::ScratchImage() };
	DirectX::LoadFromWICFile(path,
		DirectX::WIC_FLAGS_FORCE_RGB,
		nullptr,
		*scratchImage);

	// https://github.com/microsoft/DirectXTK12/wiki/ResourceUploadBatch
	DirectX::ResourceUploadBatch upload(pDevice->GetDevice());
	upload.Begin();

	std::unique_ptr<uint8_t[]> decodedData;
	D3D12_SUBRESOURCE_DATA subresource{};

	DirectX::LoadWICTextureFromFileEx(pDevice->GetDevice(), path, 0, D3D12_RESOURCE_FLAG_NONE, DirectX::DX12::WIC_LOADER_MIP_RESERVE, m_Texture.ReleaseAndGetAddressOf(), decodedData, subresource);

	const auto desc{ m_Texture->GetDesc() };

	//desc.MipLevels
	const uint32_t mips = (desc.MipLevels < m_MipLevels) ? desc.MipLevels : m_MipLevels;
	const auto uploadDesc = CD3DX12_RESOURCE_DESC(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		desc.Width, desc.Height, 1, mips,
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
	srvDesc.Texture2D.MipLevels = mips;
	srvDesc.Texture2D.MostDetailedMip = 0;

	pDevice->GetMainHeap()->Allocate(m_Descriptor);
	pDevice->GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_Descriptor.GetCPU());
	
	auto finish{ upload.End(pDevice->GetCommandQueue()) };
	finish.wait();

	//pDevice->ExecuteCommandList(true);
}

void Texture::CreateFromDDS(DeviceContext* pDevice, const std::string_view& TexturePath)
{
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
	pDevice->GetCommandList()->ResourceBarrier(1, &copyToResourceBarrier);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.MostDetailedMip = 0;

	pDevice->GetMainHeap()->Allocate(m_Descriptor);
	pDevice->GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_Descriptor.GetCPU());

	m_Texture->SetName(L"Texture SRV");
}

void Texture::CreateFromHDR(DeviceContext* pDevice, const std::string_view& TexturePath)
{
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
	//D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	const auto copyToResourceBarrier{ CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(),
																	 D3D12_RESOURCE_STATE_COPY_DEST,
																	 D3D12_RESOURCE_STATE_COMMON) };
	pDevice->GetCommandList()->ResourceBarrier(1, &copyToResourceBarrier);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	pDevice->GetMainHeap()->Allocate(m_Descriptor);
	pDevice->GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_Descriptor.GetCPU());

	m_Texture->SetName(L"HDR Texture SRV");
}

void Texture::Release()
{
	SAFE_RELEASE(m_TextureUploadHeap);
	SAFE_RELEASE(m_Texture);
}
