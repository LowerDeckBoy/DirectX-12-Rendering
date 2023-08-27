#pragma once
//#include <d3d12.h>

class DeviceContext;
class Descriptor;

class BufferUtils
{
public:
	static ID3D12Resource* Create(ID3D12Device5* pDevice, const uint64_t Size, const D3D12_RESOURCE_FLAGS Flags, const D3D12_RESOURCE_STATES InitState, const D3D12_HEAP_PROPERTIES& HeapProps, D3D12_HEAP_FLAGS HeapFlags = D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE);
	
	static void Create(ID3D12Device5* pDevice, ID3D12Resource* pTarget, const uint64_t Size, const D3D12_RESOURCE_FLAGS Flags, const D3D12_RESOURCE_STATES InitState, const D3D12_HEAP_PROPERTIES& HeapProps, D3D12_HEAP_FLAGS HeapFlags = D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE);
	
	static void Create(ID3D12Device5* pDevice, ID3D12Resource** ppTarget, const uint64_t Size, const D3D12_RESOURCE_FLAGS Flags, const D3D12_RESOURCE_STATES InitState, const D3D12_HEAP_PROPERTIES& HeapProps, D3D12_HEAP_FLAGS HeapFlags = D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE);

	static void CreateUAV(DeviceContext* pDevice, ID3D12Resource** ppTargetResource, size_t BufferSize, D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COMMON);
	
	static void CreateBufferSRV(DeviceContext* pDevice, Descriptor& DescriptorRef, ID3D12Resource* pBuffer, uint32_t BufferElements, uint32_t BufferElementSize);
	
	// Write only
	static uint8_t* MapCPU(ID3D12Resource* pResource);

};
