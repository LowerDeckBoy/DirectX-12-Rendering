#include "Buffer.hpp"
#include "../Utilities/Utilities.hpp"


Buffer::Buffer(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, BufferType TypeOf, bool bSRV)
{
	Create(pDevice, Data, Desc, TypeOf, bSRV);
}

Buffer::~Buffer()
{
	if (m_Allocation)
	{
		m_Allocation->Release();
		m_Allocation = nullptr;

	}

	if (m_UploadAllocation)
	{
		m_UploadAllocation->Release();
		m_UploadAllocation = nullptr;
	}


	SAFE_RELEASE(m_UploadHeap);
	SAFE_RELEASE(m_Buffer);
}

void Buffer::Create(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, BufferType TypeOf, bool bSRV)
{
	m_BufferData = Data;
	const auto heapDesc{ CD3DX12_RESOURCE_DESC::Buffer(Data.Size) };

	D3D12MA::ALLOCATION_DESC allocDesc{};
	allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
	//allocDesc.Flags = D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_COMMITTED | D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY;
	ID3D12Resource* bufferPtr{ nullptr };
	//pDevice->GetAllocator()->CreateResource(&allocDesc, &heapDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, &allocation, IID_PPV_ARGS(m_Buffer.ReleaseAndGetAddressOf()));
	ThrowIfFailed(pDevice->GetAllocator()->CreateResource(&allocDesc, &heapDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, &m_Allocation, IID_PPV_ARGS(&bufferPtr)));
	m_Buffer.Attach(bufferPtr);
	//https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator/blob/master/src/D3D12Sample.cpp

	D3D12MA::ALLOCATION_DESC uploadDesc{};
	uploadDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
	uploadDesc.Flags = D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_COMMITTED | D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY;
	ThrowIfFailed(pDevice->GetAllocator()->CreateResource(&uploadDesc, &heapDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, &m_UploadAllocation, IID_PPV_ARGS(m_UploadHeap.ReleaseAndGetAddressOf())));

	D3D12_SUBRESOURCE_DATA subresource{};
	subresource.pData		= Data.pData;
	subresource.RowPitch	= Data.Size;
	subresource.SlicePitch	= Data.Size;

	::UpdateSubresources(pDevice->GetCommandList(), m_Buffer.Get(), m_UploadHeap.Get(), 0, 0, 1, &subresource);
	
	if (TypeOf == BufferType::eVertex || TypeOf == BufferType::eConstant)
	{
		const auto barrier{ CD3DX12_RESOURCE_BARRIER::Transition(m_Buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER) };
		pDevice->GetCommandList()->ResourceBarrier(1, &barrier);

	}
	else if (TypeOf == BufferType::eIndex)
	{
		const auto barrier{ CD3DX12_RESOURCE_BARRIER::Transition(m_Buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER) };
		pDevice->GetCommandList()->ResourceBarrier(1, &barrier);
	}
	else
	{
		std::logic_error("Invalid buffer type!");
	}

	//MapMemory();

	if (bSRV)
	{
		pDevice->GetMainHeap()->Allocate(m_Descriptor);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = Data.ElementsCount;
		srvDesc.Buffer.StructureByteStride = Data.Stride;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		pDevice->GetDevice()->CreateShaderResourceView(m_Buffer.Get(), &srvDesc, m_Descriptor.GetCPU());
	}
}

void Buffer::MapMemory()
{
	uint8_t* pDataBegin{ nullptr };
	const CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(m_Buffer.Get()->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
	std::memcpy(pDataBegin, m_BufferData.pData, m_BufferData.Size);
	m_Buffer.Get()->Unmap(0, nullptr);
}

ID3D12Resource* Buffer::GetBuffer() const noexcept
{
	return m_Buffer.Get();
}

D3D12_GPU_VIRTUAL_ADDRESS Buffer::GetGPUAddress() const
{
	return m_Buffer.Get()->GetGPUVirtualAddress();
}

BufferData Buffer::GetData() noexcept
{
	return m_BufferData;
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

uint8_t* BufferUtils::MapCPU(ID3D12Resource* pResource)
{
	uint8_t* pMapped{ nullptr };
	const CD3DX12_RANGE range(0, 0);
	ThrowIfFailed(pResource->Map(0, &range, reinterpret_cast<void**>(&pMapped)));

	return pMapped;
}
