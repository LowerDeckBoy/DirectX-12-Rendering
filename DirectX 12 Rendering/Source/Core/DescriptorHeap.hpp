#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>

class Descriptor
{
private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{};
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle{};
	
public:
	// Setter for CPU descriptor  handle
	[[maybe_unused]] void SetCPU(D3D12_CPU_DESCRIPTOR_HANDLE Handle) noexcept { m_cpuHandle = Handle; }
	// Setter for GPU descriptor  handle
	[[maybe_unused]] void SetGPU(D3D12_GPU_DESCRIPTOR_HANDLE Handle)noexcept { m_gpuHandle = Handle; }

	// Get CPU descriptor handle
	[[nodiscard]] inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPU() const noexcept { return m_cpuHandle; }

	// Get GPU descriptor handle
	[[nodiscard]] inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPU() const noexcept { return m_gpuHandle; }

	constexpr bool bIsValid() const { return m_cpuHandle.ptr != 0; }

	int32_t m_Index{};
};

class DescriptorHeap
{
public:
	~DescriptorHeap() noexcept(false)
	{
		m_Heap.Reset();
		m_Heap = nullptr;
	}

	void Allocate(Descriptor& TargetDescriptor)
	{
		if (TargetDescriptor.bIsValid())
		{
			Override(TargetDescriptor);
		}
		else
		{
			TargetDescriptor.SetCPU(GetCPUptr(m_Allocated));
			TargetDescriptor.SetGPU(GetGPUptr(m_Allocated));
			TargetDescriptor.m_Index = m_Allocated;
			m_Allocated++;
		}
	}

	void Override(Descriptor& TargetDescriptor)
	{
		TargetDescriptor.SetCPU(GetCPUptr(TargetDescriptor.m_Index));
		TargetDescriptor.SetGPU(GetGPUptr(TargetDescriptor.m_Index));
	}

	// TODO
	//void Release(Descriptor& TargetDescriptor)
	//{
	//	m_Allocated--;
	//}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUptr(uint32_t Index)
	{
		return { m_Heap.Get()->GetCPUDescriptorHandleForHeapStart().ptr + Index * m_DescriptorSize };
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUptr(uint32_t Index)
	{
		return { m_Heap.Get()->GetGPUDescriptorHandleForHeapStart().ptr + Index * m_DescriptorSize };
	}

	inline ID3D12DescriptorHeap* GetHeap() const { return m_Heap.Get(); }
	inline ID3D12DescriptorHeap** GetHeapAddressOf() { return m_Heap.GetAddressOf(); }

	[[maybe_unused]] inline uint32_t GetDescriptorSize() const { return m_DescriptorSize; }
	[[maybe_unused]] inline uint32_t GetDescriptorsCount() const { return m_NumDescriptors; }
	[[maybe_unused]] inline uint32_t GetAllocatedCount() const { return m_Allocated; }

	void SetDescriptorSize(uint32_t NewSize)
	{
		m_DescriptorSize = NewSize;
	}
	void SetDescriptorsCount(uint32_t Count)
	{
		m_NumDescriptors = Count;
	}

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
	D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
	uint32_t m_DescriptorSize{ 32 };
	uint32_t m_NumDescriptors{ 0 };
	uint32_t m_Allocated{ 0 };

};

