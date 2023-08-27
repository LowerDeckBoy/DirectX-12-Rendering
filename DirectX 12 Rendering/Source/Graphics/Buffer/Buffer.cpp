#include "Buffer.hpp"
#include "Utilities.hpp"


Buffer::Buffer(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, BufferType TypeOf, bool bSRV)
{
	Create(pDevice, Data, Desc, TypeOf, bSRV);
}

Buffer::~Buffer()
{
	SAFE_RELEASE(m_Buffer);
	if (m_Allocation)
	{
		m_Allocation->Release();
		m_Allocation = nullptr;
	}
}

void Buffer::Create(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, BufferType TypeOf, bool bSRV)
{
	m_BufferData = Data;
	const auto heapDesc{ CD3DX12_RESOURCE_DESC::Buffer(Data.Size) };

	D3D12MA::ALLOCATION_DESC allocDesc{};
	allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
	allocDesc.Flags = D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY;
	allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
	ThrowIfFailed(pDevice->GetAllocator()->CreateResource(&allocDesc, &heapDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, &m_Allocation, IID_PPV_ARGS(m_Buffer.ReleaseAndGetAddressOf())));

	D3D12MA::ALLOCATION_DESC uploadDesc{};
	uploadDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
	uploadDesc.Flags = D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_COMMITTED | D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY | D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_BEST_FIT;
	uploadDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;

	D3D12MA::Allocation* uploadHeapAllocation{ nullptr };
	ID3D12Resource* uploadHeap{ nullptr };
	ThrowIfFailed(pDevice->GetAllocator()->CreateResource(&uploadDesc, &heapDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, &uploadHeapAllocation, IID_PPV_ARGS(&uploadHeap)));

	D3D12_SUBRESOURCE_DATA subresource{};
	subresource.pData		= Data.pData;
	subresource.RowPitch	= Data.Size;
	subresource.SlicePitch	= Data.Size;

	::UpdateSubresources(pDevice->GetCommandList(), m_Buffer.Get(), uploadHeap, 0, 0, 1, &subresource);
	
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
		throw std::logic_error("Invalid buffer type!");

	pDevice->ExecuteCommandList(true);

	uploadHeapAllocation->Release();

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

VertexBuffer::VertexBuffer(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, bool bSRV)
{
	Buffer::Create(pDevice, Data, Desc, BufferType::eVertex, bSRV);
	SetView();
}

void VertexBuffer::Create(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, bool bSRV)
{
	Buffer::Create(pDevice, Data, Desc, BufferType::eVertex, bSRV);
	SetView();
}

void VertexBuffer::SetView()
{
	View.BufferLocation = Buffer::GetGPUAddress();
	View.SizeInBytes = static_cast<uint32_t>(Buffer::GetData().Size);
	View.StrideInBytes = static_cast<uint32_t>(Buffer::GetData().Size) / Buffer::GetData().ElementsCount;
}

IndexBuffer::IndexBuffer(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, bool bSRV)
{
	Buffer::Create(pDevice, Data, Desc, BufferType::eIndex, bSRV);
	SetView();
}

void IndexBuffer::Create(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, bool bSRV)
{
	Buffer::Create(pDevice, Data, Desc, BufferType::eIndex, bSRV);
	SetView();
}

void IndexBuffer::SetView()
{
	View.BufferLocation = Buffer::GetGPUAddress();
	View.Format = DXGI_FORMAT_R32_UINT;
	View.SizeInBytes = static_cast<uint32_t>(Buffer::GetData().Size);
	Count = Buffer::GetData().ElementsCount;
}
