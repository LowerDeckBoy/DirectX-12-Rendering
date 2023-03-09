#pragma once
#include <d3d12.h>
#include <D3D12MA/D3D12MemAlloc.h>
#include <wrl.h>
#include <memory>
#include <deque>

constexpr uint32_t BLOCK_ALIGNMENT{ 256 };

// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/LinearAllocator.h
// https://www.3dgep.com/learning-directx-12-3/
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12Raytracing/src/D3D12RaytracingHelloWorld/D3D12RaytracingHelloWorld.h
class UploadBuffer
{
public:
	//UploadBuffer();
	//~UploadBuffer();

	//void Allocate();
	//void Release();

	void* m_cpuData{ nullptr };
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle{};

	//D3D12MA::Allocation

};

