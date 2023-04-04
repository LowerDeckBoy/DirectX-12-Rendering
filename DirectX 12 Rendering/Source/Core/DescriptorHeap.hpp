#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <cstdint>

struct Descriptor
{
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{};
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle{};

	constexpr bool bIsValid() const { return m_cpuHandle.ptr != 0; }
};

class DescriptorHeap
{
public:

	void Allocate(Descriptor& targetDescriptor)
	{
		targetDescriptor.m_cpuHandle = GetCPU(m_Allocated);
		targetDescriptor.m_gpuHandle = GetGPU(m_Allocated);
		m_Allocated++;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPU(uint32_t Index)
	{
		return D3D12_CPU_DESCRIPTOR_HANDLE(m_Heap.Get()->GetCPUDescriptorHandleForHeapStart().ptr + Index * m_DescriptorSize);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPU(uint32_t Index)
	{
		return D3D12_GPU_DESCRIPTOR_HANDLE(m_Heap.Get()->GetGPUDescriptorHandleForHeapStart().ptr + Index * m_DescriptorSize);
	}


	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
	//D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
	uint32_t m_DescriptorSize{ 32 };
	uint32_t m_NumDescriptors{ 0 };
	uint32_t m_Allocated{ 0 };
private:

};

