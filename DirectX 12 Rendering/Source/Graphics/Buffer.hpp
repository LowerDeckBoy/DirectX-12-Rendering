#pragma once
#include "Vertex.hpp"
#include "../Utils/Utilities.hpp"
#include "../Core/Device.hpp"


enum class BufferType : uint8_t
{
	eVertex = 0,
	eIndex,
	eConstant
};

struct BufferData
{
	BufferData() {}
	BufferData(void* pData, size_t Count, size_t Size) :
		pData(pData), ElementsCount(static_cast<uint32_t>(Count)), Size(Size)
	{ }

	void*	 pData{ nullptr };
	uint32_t ElementsCount{ 0 };
	size_t	 Size{ 0 };
	//BufferType TypeOf;
};

// Default properties
// Properties:	Upload
// Flags:		None
// State:		Common
// Format:		Unknown
struct BufferDesc
{
	CD3DX12_HEAP_PROPERTIES HeapProperties{ D3D12_HEAP_TYPE_UPLOAD };
	D3D12_RESOURCE_STATES	State{ D3D12_RESOURCE_STATE_COMMON };
	D3D12_HEAP_FLAGS		HeapFlags{ D3D12_HEAP_FLAG_NONE };
	DXGI_FORMAT				Format{ DXGI_FORMAT_UNKNOWN };
};

class Buffer
{
public:
	void Create(Device* pDevice, BufferData Data, BufferDesc Desc)
	{
		m_DeviceCtx = pDevice;
		m_BufferDesc = Desc;
		m_BufferData = Data;

		auto heapDesc{ CD3DX12_RESOURCE_DESC::Buffer(Data.Size) };

		ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(
			&Desc.HeapProperties,
			Desc.HeapFlags,
			&heapDesc,
			Desc.State,
			nullptr,
			IID_PPV_ARGS(m_Buffer.GetAddressOf())));

		MapMemory();

		pDevice->GetMainHeap().Allocate(m_Descriptor);
	}

	void MapMemory()
	{
		uint8_t* pDataBegin{ nullptr };
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(m_Buffer.Get()->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
		std::memcpy(pDataBegin, m_BufferData.pData, m_BufferData.Size);
		m_Buffer.Get()->Unmap(0, nullptr);
	}

	ID3D12Resource* GetBuffer() const
	{
		return m_Buffer.Get();
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const
	{
		return m_Buffer.Get()->GetGPUVirtualAddress();
	}

	BufferDesc GetDesc() { return m_BufferDesc; }
	BufferData GetData() { return m_BufferData; }

	Descriptor m_Descriptor{};
	Descriptor DescriptorSRV{};

protected:
	Device* m_DeviceCtx{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12Resource> m_BufferUploadHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_Buffer;

	BufferDesc m_BufferDesc{};
	BufferData m_BufferData{};
};

class VertexBuffer
{
public:
	VertexBuffer() {}
	VertexBuffer(Device* pDevice, BufferData Data, BufferDesc Desc) 
	{
		Buffer.Create(pDevice, Data, Desc); 
		SetView();
	}

	Buffer Buffer;
	D3D12_VERTEX_BUFFER_VIEW View{};

	void SetView()
	{
		View.BufferLocation	= this->Buffer.GetGPUAddress();
		View.SizeInBytes	= static_cast<uint32_t>(this->Buffer.GetData().Size);
		View.StrideInBytes	= static_cast<uint32_t>(this->Buffer.GetData().Size) / this->Buffer.GetData().ElementsCount;
	}
};

class IndexBuffer
{
public:
	IndexBuffer() {}
	IndexBuffer(Device* pDevice, BufferData Data, BufferDesc Desc)
	{ 
		Buffer.Create(pDevice, Data, Desc);
		SetView();
	}

	Buffer Buffer;
	D3D12_INDEX_BUFFER_VIEW View{};
	uint32_t Count{ 0 };

	void SetView()
	{
		View.BufferLocation = this->Buffer.GetGPUAddress();
		View.Format			= DXGI_FORMAT_R32_UINT;
		View.SizeInBytes	= static_cast<uint32_t>(this->Buffer.GetData().Size);
		Count				= this->Buffer.GetData().ElementsCount;
	}
};

class BufferUtils
{
public:
	//static BufferDesc DefaultDesc();
	//static BufferDesc UploadDesc();

	//static ID3D12Resource* CreateSRV();
	//static ID3D12Resource* CreateUAV();


};
