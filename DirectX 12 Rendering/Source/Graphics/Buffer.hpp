#pragma once
#include "Vertex.hpp"
#include "../Core/DeviceContext.hpp"

struct BufferData
{
	BufferData() {}
	BufferData(void* pData, size_t Count, size_t Size, size_t Stride) :
		pData(pData), ElementsCount(static_cast<uint32_t>(Count)), Size(Size), Stride(static_cast<uint32_t>(Stride))
	{ }

	void*	 pData{ nullptr };
	uint32_t ElementsCount{ 0 };
	size_t	 Size{ 0 };
	uint32_t Stride{ 0 };
	//BufferType TypeOf;
};

// Default properties
// Properties:	Upload
// Flags:		None
// State:		Generic Read
// Format:		Unknown
struct BufferDesc
{
	CD3DX12_HEAP_PROPERTIES HeapProperties{ D3D12_HEAP_TYPE_UPLOAD };
	D3D12_RESOURCE_STATES	State{ D3D12_RESOURCE_STATE_GENERIC_READ };
	D3D12_HEAP_FLAGS		HeapFlags{ D3D12_HEAP_FLAG_NONE };
	DXGI_FORMAT				Format{ DXGI_FORMAT_UNKNOWN };
};

enum class BufferType : uint8_t
{
	eVertex = 0,
	eIndex,
	eConstant
};

class Buffer
{
public:
	Buffer() noexcept { }
	Buffer(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, BufferType TypeOf, bool bSRV);
	~Buffer();

	void Create(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, BufferType TypeOf, bool bSRV);

	void MapMemory();

	ID3D12Resource* GetBuffer() const noexcept;
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const;
	
	BufferData GetData() noexcept;

	Descriptor m_Descriptor{};
	Descriptor DescriptorSRV{};

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_Buffer;
	D3D12MA::Allocation* m_Allocation{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadHeap;
	D3D12MA::Allocation* m_UploadAllocation{ nullptr };

	BufferData m_BufferData{};
};

class VertexBuffer
{
public:
	VertexBuffer() {}
	VertexBuffer(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, bool bSRV = false)
	{
		Buffer.Create(pDevice, Data, Desc, BufferType::eVertex, bSRV);
		SetView();
	}

	void Create(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, bool bSRV = false)
	{
		Buffer.Create(pDevice, Data, Desc, BufferType::eVertex, bSRV);
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
	IndexBuffer(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, bool bSRV = false)
	{ 
		Buffer.Create(pDevice, Data, Desc, BufferType::eIndex, bSRV);
		SetView();
	}

	void Create(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, bool bSRV = false)
	{
		Buffer.Create(pDevice, Data, Desc, BufferType::eIndex, bSRV);
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
	static void Create(ID3D12Device5* pDevice, ID3D12Resource** ppTarget, const uint64_t Size, const D3D12_RESOURCE_FLAGS Flags, const D3D12_RESOURCE_STATES InitState, const D3D12_HEAP_PROPERTIES& HeapProps, D3D12_HEAP_FLAGS HeapFlags = D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE);
	
	// Write only
	static uint8_t* MapCPU(ID3D12Resource* pResource);

};
