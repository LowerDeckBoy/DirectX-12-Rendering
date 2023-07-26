#include "../Core/DeviceContext.hpp"
#include "../Core/DescriptorHeap.hpp"
#include "TextureUtils.hpp"
#include <DirectXTex.h>
#include <directxtk12/WICTextureLoader.h>
#include <directxtk12/DDSTextureLoader.h>
#include <directxtk12/ResourceUploadBatch.h>
#include "../Utilities/Utilities.hpp"
#include "../Utilities/FileUtils.hpp"

ID3D12Resource* TextureUtils::CreateResource(ID3D12Device5* pDevice, TextureDesc Desc, TextureData Data)
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Flags = Desc.Flag;
	resourceDesc.Format = Data.Format;
	resourceDesc.Width = Data.Width;
	resourceDesc.Height = Data.Height;
	resourceDesc.Dimension = Data.Dimension;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = Data.Depth;
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

void TextureUtils::CreateSRV(ID3D12Device5* pDevice, ID3D12Resource* pResource, Descriptor& TargetDescriptor, DXGI_FORMAT Format, uint16_t Depth)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = Format;
	if (Depth == 1)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
	}
	else if (Depth == 6)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = 1;
	}

	pDevice->CreateShaderResourceView(pResource, &srvDesc, TargetDescriptor.GetCPU());

}

void TextureUtils::CreateUAV(ID3D12Device5* pDevice, ID3D12Resource* pResource, Descriptor& TargetDescriptor, DXGI_FORMAT Format, uint16_t Depth)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = Format;
	if (Depth == 1)
	{
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
	}
	else if (Depth == 6)
	{
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY; 
		uavDesc.Texture2DArray.MipSlice = 0;
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		uavDesc.Texture2DArray.ArraySize = Depth;
	}

	pDevice->CreateUnorderedAccessView(pResource, nullptr, &uavDesc, TargetDescriptor.GetCPU());
}

ID3D12Resource* TextureUtils::CreateFromHDR(DeviceContext* pDeviceContext, const std::string_view& Filepath)
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

	// Output resource
	ID3D12Resource* pResource{ nullptr };
	ThrowIfFailed(pDeviceContext->GetDevice()->CreateCommittedResource(
		&HeapProps::HeapDefault,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&pResource)));

	const uint64_t bufferSize{ ::GetRequiredIntermediateSize(pResource, 0, 1) };

	ID3D12Resource* uploadHeap{ nullptr };
	const auto bufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize) };
	ThrowIfFailed(pDeviceContext->GetDevice()->CreateCommittedResource(
		&HeapProps::HeapUpload,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadHeap)));


	D3D12_SUBRESOURCE_DATA subresource{};
	subresource.pData = scratchImage.GetImages()->pixels;
	subresource.RowPitch = static_cast<LONG_PTR>(scratchImage.GetImages()->rowPitch);
	subresource.SlicePitch = static_cast<LONG_PTR>(scratchImage.GetImages()->slicePitch);

	::UpdateSubresources(pDeviceContext->GetCommandList(), pResource, uploadHeap, 0, 0, 1, &subresource);

	const auto copyToResourceBarrier{ CD3DX12_RESOURCE_BARRIER::Transition(pResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ) };

	pDeviceContext->GetCommandList()->ResourceBarrier(1, &copyToResourceBarrier);

	return pResource;
}
