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
	eConstant,
	eStructured
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

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_Buffer;
	D3D12MA::Allocation* m_Allocation{ nullptr };

	BufferData m_BufferData{};
};

class VertexBuffer : public Buffer
{
public:
	VertexBuffer() {}
	VertexBuffer(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, bool bSRV = false);

	void Create(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, bool bSRV = false);
	
	D3D12_VERTEX_BUFFER_VIEW View{};

private:
	void SetView();
};

class IndexBuffer : public Buffer
{
public:
	IndexBuffer() {}
	IndexBuffer(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, bool bSRV = false);

	void Create(DeviceContext* pDevice, BufferData Data, BufferDesc Desc, bool bSRV = false);

	D3D12_INDEX_BUFFER_VIEW View{};
	uint32_t Count{ 0 };

private:
	void SetView();
};
