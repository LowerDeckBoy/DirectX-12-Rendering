#include "../Core/DeviceContext.hpp"
#include "TextureUtils.hpp"
#include <DirectXTex.h>
#include <directxtk12/WICTextureLoader.h>
#include <directxtk12/DDSTextureLoader.h>
#include <directxtk12/ResourceUploadBatch.h>
#include "../Core/DescriptorHeap.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Utilities/FileUtils.hpp"


void TextureUtils::CreateResource(ID3D12Device* pDevice, ID3D12Resource* pTargetResource, TextureDesc Desc, TextureData Data)
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Format = Data.Format;
	resourceDesc.Width = Data.Width;
	resourceDesc.Height = Data.Height;
	resourceDesc.Dimension = Data.Dimension;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.SampleDesc = { 1, 0 };

	ThrowIfFailed(pDevice->CreateCommittedResource(
		&Desc.HeapProperties,
		Desc.HeapFlag,
		&resourceDesc,
		Desc.State,
		nullptr,
		IID_PPV_ARGS(&pTargetResource)
	));
}

ID3D12Resource* TextureUtils::CreateResource(ID3D12Device* pDevice, TextureDesc Desc, TextureData Data)
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Flags = Desc.Flag;
	resourceDesc.Format = Data.Format;
	resourceDesc.Width = Data.Width;
	resourceDesc.Height = Data.Height;
	resourceDesc.Dimension = Data.Dimension;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.SampleDesc = { 1, 0 };

	ID3D12Resource* resource{ nullptr };
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&Desc.HeapProperties,
		Desc.HeapFlag,
		&resourceDesc,
		Desc.State,
		nullptr,
		IID_PPV_ARGS(&resource)
	));

	return resource;
}

ID3D12Resource* TextureUtils::CreateResource(ID3D12Device* pDevice, CD3DX12_RESOURCE_DESC& Desc)
{



	return nullptr;
}

void TextureUtils::CreateFromWIC(ID3D12Device* pDevice, ID3D12Resource* pTargetResource, const std::string_view& Filepath, TextureDesc Desc)
{

	DirectX::ScratchImage scratchImage;
	std::wstring wpath = std::wstring(Filepath.begin(), Filepath.end());
	DirectX::LoadFromWICFile(wpath.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, scratchImage);
	DirectX::TexMetadata metadata{ scratchImage.GetMetadata() };

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Flags = Desc.Flag;
	resourceDesc.MipLevels = static_cast<uint16_t>(metadata.mipLevels);
	resourceDesc.Width = static_cast<uint64_t>(metadata.width);
	resourceDesc.Height	= static_cast<uint32_t>(metadata.height);
	resourceDesc.DepthOrArraySize = static_cast<uint16_t>(metadata.depth);
	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc = { 1, 0 };

	if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE1D)
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE2D)
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;

	ThrowIfFailed(pDevice->CreateCommittedResource(
		&Desc.HeapProperties,
		Desc.HeapFlag,
		&resourceDesc,
		Desc.State,
		nullptr,
		IID_PPV_ARGS(&pTargetResource)
	));

}

ID3D12Resource* TextureUtils::CreateFromWIC(ID3D12Device* pDevice, const std::string_view& Filepath, TextureDesc Desc)
{
	DirectX::ScratchImage scratchImage;
	std::wstring wpath = std::wstring(Filepath.begin(), Filepath.end());
	DirectX::LoadFromWICFile(wpath.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, scratchImage);
	DirectX::TexMetadata metadata{ scratchImage.GetMetadata() };

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Flags = Desc.Flag;
	resourceDesc.MipLevels = static_cast<uint16_t>(metadata.mipLevels);
	resourceDesc.Width = static_cast<uint64_t>(metadata.width);
	resourceDesc.Height = static_cast<uint32_t>(metadata.height);
	resourceDesc.DepthOrArraySize = static_cast<uint16_t>(metadata.depth);
	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc = { 1, 0 };

	if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE1D)
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE2D)
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;

	ID3D12Resource* resource{ nullptr };
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&Desc.HeapProperties,
		Desc.HeapFlag,
		&resourceDesc,
		Desc.State,
		nullptr,
		IID_PPV_ARGS(&resource)
	));

	return resource;
}

ID3D12Resource* TextureUtils::CreateFromWIC(DeviceContext* pDevice, ID3D12Resource** ppTarget, const std::string_view& Filepath, TextureDesc Desc)
{
	std::wstring wpath = std::wstring(Filepath.begin(), Filepath.end());
	const wchar_t* path = wpath.c_str();

	DirectX::ScratchImage* scratchImage{ new DirectX::ScratchImage() };
	DirectX::LoadFromWICFile(path,
		DirectX::WIC_FLAGS_NONE,
		nullptr,
		*scratchImage);

	DirectX::ResourceUploadBatch upload(pDevice->GetDevice());
	upload.Begin();

	std::unique_ptr<uint8_t[]> decodedData;
	D3D12_SUBRESOURCE_DATA subresource{};

	DirectX::LoadWICTextureFromFileEx(pDevice->GetDevice(), path, 0, D3D12_RESOURCE_FLAG_NONE, DirectX::DX12::WIC_LOADER_MIP_AUTOGEN, &(*(ppTarget)), decodedData, subresource);

	auto desc{ (*ppTarget)->GetDesc() };

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
		IID_PPV_ARGS(&(*(ppTarget)))));

	upload.Upload((*ppTarget), 0, &subresource, 1);
	upload.Transition((*ppTarget), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	upload.GenerateMips((*ppTarget));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	//m_Device->GetMainHeap()->Allocate(m_Descriptor);
	//m_Device->GetDevice()->CreateShaderResourceView((*ppTarget), &srvDesc, m_Descriptor.GetCPU());

	auto finish{ upload.End(pDevice->GetCommandQueue()) };
	finish.wait();


	return nullptr;
}

ID3D12Resource* TextureUtils::CreateFromDDS(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, const std::string_view& Filepath, TextureDesc Desc)
{
	if (files::GetExtension(Filepath.data()) != ".dds")
		throw std::logic_error("Invalid texture extension!");

	std::wstring wpath{ std::wstring(Filepath.begin(), Filepath.end()) };

	DirectX::ScratchImage scratchImage{};
	DirectX::LoadFromDDSFile(wpath.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, scratchImage);
	DirectX::TexMetadata metadata{ scratchImage.GetMetadata() };

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Flags = Desc.Flag;
	resourceDesc.MipLevels = static_cast<uint16_t>(metadata.mipLevels);
	resourceDesc.Width = static_cast<uint64_t>(metadata.width);
	resourceDesc.Height = static_cast<uint32_t>(metadata.height);
	resourceDesc.DepthOrArraySize = metadata.IsCubemap() ? static_cast<uint16_t>(metadata.arraySize) : static_cast<uint16_t>(metadata.depth);
	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc = { 1, 0 };

	if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE1D)
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE2D)
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;

	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	for (uint8_t i = 0; i < 6; i++)
	{
		auto image{ scratchImage.GetImage(0, i, 0) };
		subresources.emplace_back(image->pixels, static_cast<LONG_PTR>(image->rowPitch), static_cast<LONG_PTR>(image->slicePitch));
	}

	ID3D12Resource* resource{ nullptr };
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&Desc.HeapProperties,
		Desc.HeapFlag,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&resource)));

	const uint64_t bufferSize{ GetRequiredIntermediateSize(resource, 0, static_cast<uint32_t>(subresources.size())) };

	Microsoft::WRL::ComPtr<ID3D12Resource> uploadHeap;

	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	const auto bufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize) };
	ThrowIfFailed(pDevice->CreateCommittedResource(&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(uploadHeap.GetAddressOf())));;

	D3D12_SUBRESOURCE_DATA subresource{
		scratchImage.GetImages()->pixels,
		static_cast<LONG_PTR>(scratchImage.GetImages()->rowPitch),
		static_cast<LONG_PTR>(scratchImage.GetImages()->slicePitch)
	};

	UpdateSubresources(pCommandList, resource, uploadHeap.Get(), 0, 0, static_cast<uint32_t>(subresources.size()), subresources.data());

	const auto copyToResourceBarrier{ CD3DX12_RESOURCE_BARRIER::Transition(resource,
																	 D3D12_RESOURCE_STATE_COPY_DEST,
																	 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) };
	pCommandList->ResourceBarrier(1, &copyToResourceBarrier);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.MostDetailedMip = 0;

	return resource;
}

ID3D12Resource* TextureUtils::CreateFromHDR(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, const std::string_view& Filepath, TextureDesc Desc)
{
	if (files::GetExtension(Filepath.data()) != ".hdr")
		throw std::logic_error("Invalid texture extension!");

	DirectX::ScratchImage scratchImage;
	std::wstring wpath = std::wstring(Filepath.begin(), Filepath.end());
	DirectX::LoadFromHDRFile(wpath.c_str(), nullptr, scratchImage);
	DirectX::TexMetadata metadata{ scratchImage.GetMetadata() };

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

	ID3D12Resource* resource{ nullptr };
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&Desc.HeapProperties,
		Desc.HeapFlag,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&resource)));

	const uint64_t bufferSize{ GetRequiredIntermediateSize(resource, 0, 1) };

	Microsoft::WRL::ComPtr<ID3D12Resource> uploadHeap;
	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	const auto bufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize) };
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&heapProperties,
		Desc.HeapFlag,
		&bufferDesc,
		Desc.State,
		nullptr,
		IID_PPV_ARGS(uploadHeap.GetAddressOf())));


	D3D12_SUBRESOURCE_DATA subresource{
		scratchImage.GetImages()->pixels,
		static_cast<LONG_PTR>(scratchImage.GetImages()->rowPitch),
		static_cast<LONG_PTR>(scratchImage.GetImages()->slicePitch)
	};

	auto commandList = pCommandList;

	UpdateSubresources(commandList, resource, uploadHeap.Get(), 0, 0, 1, &subresource);

	const auto copyToResourceBarrier{ CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) };
	//D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)};
	commandList->ResourceBarrier(1, &copyToResourceBarrier);
	//commandList->CopyResource()

	SAFE_RELEASE(uploadHeap);

	return resource;
}

void TextureUtils::CreateTextureSRV(ID3D12Resource* pResource, Descriptor& Descriptor)
{
}
