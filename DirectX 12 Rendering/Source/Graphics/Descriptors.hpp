#pragma once

#include <d3d12.h>


class Descriptors
{
public:

	D3D12_DESCRIPTOR_HEAP_DESC CreateHeapDesc(D3D12_DESCRIPTOR_HEAP_TYPE Type, D3D12_DESCRIPTOR_HEAP_FLAGS Flags, uint32_t Count)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Type = Type;
		desc.Flags = Flags;
		desc.NumDescriptors = Count;

		return desc;
	}

	//D3D12_DESCRIPTOR_HEAP_DESC CreateSRVHeap();

private:

public:

};