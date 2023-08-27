#include "../../Core/DeviceContext.hpp"
#include "BufferUtils.hpp"
#include "../../Core/DescriptorHeap.hpp"
#include "../../Utilities/Utilities.hpp"

ID3D12Resource* BufferUtils::Create(ID3D12Device5* pDevice, const uint64_t Size, const D3D12_RESOURCE_FLAGS Flags, const D3D12_RESOURCE_STATES InitState, const D3D12_HEAP_PROPERTIES& HeapProps, D3D12_HEAP_FLAGS HeapFlags)
{

	D3D12_RESOURCE_DESC desc{ };
	desc.Flags = Flags;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.MipLevels = 1;
	desc.DepthOrArraySize = 1;
	desc.Height = 1;
	desc.Width = Size;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.SampleDesc = { 1, 0 };

	ID3D12Resource* buffer{ nullptr };

	ThrowIfFailed(pDevice->CreateCommittedResource(&HeapProps, HeapFlags, &desc, InitState, nullptr, IID_PPV_ARGS(&buffer)));

	return buffer;
}

void BufferUtils::Create(ID3D12Device5* pDevice, ID3D12Resource* pTarget, const uint64_t Size, const D3D12_RESOURCE_FLAGS Flags, const D3D12_RESOURCE_STATES InitState, const D3D12_HEAP_PROPERTIES& HeapProps, D3D12_HEAP_FLAGS HeapFlags)
{

	D3D12_RESOURCE_DESC desc{ };
	desc.Flags = Flags;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.MipLevels = 1;
	desc.DepthOrArraySize = 1;
	desc.Height = 1;
	desc.Width = Size;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.SampleDesc = { 1, 0 };

	ID3D12Resource* result{ nullptr };
	ThrowIfFailed(pDevice->CreateCommittedResource(&HeapProps, HeapFlags, &desc, InitState, nullptr, IID_PPV_ARGS(&result)));
	if (result)
		pTarget = result;

}

void BufferUtils::Create(ID3D12Device5* pDevice, ID3D12Resource** ppTarget, const uint64_t Size, const D3D12_RESOURCE_FLAGS Flags, const D3D12_RESOURCE_STATES InitState, const D3D12_HEAP_PROPERTIES& HeapProps, D3D12_HEAP_FLAGS HeapFlags)
{
	D3D12_RESOURCE_DESC desc{};
	desc.Flags = Flags;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.MipLevels = 1;
	desc.DepthOrArraySize = 1;
	desc.Height = 1;
	desc.Width = Size;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.SampleDesc = { 1, 0 };

	ThrowIfFailed(pDevice->CreateCommittedResource(&HeapProps, HeapFlags, &desc, InitState, nullptr, IID_PPV_ARGS(&(*(ppTarget)))));
}

void BufferUtils::CreateUAV(DeviceContext* pDevice, ID3D12Resource** ppTargetResource, size_t BufferSize, D3D12_RESOURCE_STATES InitialState)
{
	const auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	const auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(BufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		InitialState,
		nullptr,
		IID_PPV_ARGS(ppTargetResource)));
}

void BufferUtils::CreateBufferSRV(DeviceContext* pDevice, Descriptor& DescriptorRef, ID3D12Resource* pBuffer, uint32_t BufferElements, uint32_t BufferElementSize)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = BufferElements;
	if (BufferElementSize == 0)
	{
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		srvDesc.Buffer.StructureByteStride = 0;
	}
	else
	{
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.StructureByteStride = BufferElementSize;
	}

	pDevice->GetMainHeap()->Allocate(DescriptorRef);
	pDevice->GetDevice()->CreateShaderResourceView(pBuffer, &srvDesc, DescriptorRef.GetCPU());

}

uint8_t* BufferUtils::MapCPU(ID3D12Resource* pResource)
{
	// temporal
	if (!pResource)
		return nullptr;

	uint8_t* pMapped{ nullptr };
	const CD3DX12_RANGE range(0, 0);
	ThrowIfFailed(pResource->Map(0, &range, reinterpret_cast<void**>(&pMapped)));

	return pMapped;
}