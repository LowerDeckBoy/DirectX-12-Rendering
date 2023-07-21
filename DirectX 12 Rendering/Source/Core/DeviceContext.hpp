#pragma once
#include <d3d12.h>
#include <d3dx12.h> // 3rd-party
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <cstdint>
#include <array>

#include "DescriptorHeap.hpp"

#include <D3D12MA/D3D12MemAlloc.h>

class Window;

using Microsoft::WRL::ComPtr;

class DeviceContext
{
public:
	DeviceContext() = default;
	DeviceContext(const DeviceContext&) = delete;
	DeviceContext operator=(const DeviceContext&) = delete;
	~DeviceContext();

	static constexpr uint32_t FRAME_COUNT{ 3 };
	static uint32_t FRAME_INDEX;

	bool Initialize();

	void CreateDevice();
	void CreateSwapChain();
	void CreateBackbuffer();

	DXGI_FORMAT GetRTVFormat() const noexcept;

	void CreateDescriptorHeaps();
	void CreateCommandList(ID3D12PipelineState* pPipelineState = nullptr);
	void CreateCommandQueue();
	void CreateFences(D3D12_FENCE_FLAGS Flags = D3D12_FENCE_FLAG_NONE);
	void CreateDepthStencil();

	void SetViewport();

	void FlushGPU();
	void MoveToNextFrame();
	void WaitForGPU();
	void ExecuteCommandList();

	// Release and recreate size dependent resources
	void OnResize();

	void Release();

public:
	// Getters
	IDXGIFactory4* GetFactory() const noexcept;
	ID3D12Device5* GetDevice() noexcept;
	IDXGISwapChain3* GetSwapChain() noexcept;

	// Command getters
	ID3D12CommandAllocator* GetCommandAllocator() const;
	ID3D12CommandQueue* GetCommandQueue() const noexcept;
	ID3D12GraphicsCommandList4* GetCommandList() noexcept;

	ID3D12CommandQueue* GetComputeQueue() const noexcept;

	// Frame/Fence getters
	uint32_t GetFrameIndex();
	ID3D12Fence1* GetFence() const noexcept;
	HANDLE& GetFenceEvent() noexcept;

	ID3D12Resource* GetRenderTarget() const noexcept;
	ID3D12DescriptorHeap* GetRenderTargetHeap() const noexcept;
	uint32_t GetDescriptorSize() const  noexcept;

	D3D12_VIEWPORT GetViewport() const noexcept;
	D3D12_RECT GetScissor() const noexcept;

	ID3D12DescriptorHeap* GetDepthHeap() noexcept;
	ID3D12Resource* GetDepthStencil() const noexcept;
	DXGI_FORMAT GetDepthFormat() const noexcept;
	Descriptor GetDepthDescriptor() const noexcept;

	// For window resizing
	void ReleaseRenderTargets();

	[[maybe_unused]]
	ID3D12DescriptorHeap* GetGUIHeap() const noexcept;

	DescriptorHeap* GetMainHeap() noexcept;

	D3D12MA::Allocator* GetAllocator() const noexcept;


private:
	ComPtr<IDXGIFactory4> m_Factory;
	ComPtr<ID3D12Device5> m_Device;
	ComPtr<ID3D12DebugDevice> m_DebugDevice;

	std::array<ComPtr<ID3D12CommandAllocator>, FRAME_COUNT> m_CommandAllocators;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12GraphicsCommandList4> m_CommandList;

	ComPtr<ID3D12CommandQueue> m_ComputeQueue;

	ComPtr<IDXGISwapChain3> m_SwapChain;
	ComPtr<ID3D12DescriptorHeap> m_RenderTargetHeap;
	std::array<ComPtr<ID3D12Resource>, FRAME_COUNT> m_RenderTargets;
	uint32_t m_DescriptorSize{ 0 };

	DXGI_FORMAT m_RenderTargetFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };

	ComPtr<ID3D12Fence1> m_Fence;
	HANDLE m_FenceEvent{};
	std::array<uint64_t, FRAME_COUNT> m_FenceValues;

	// TODO: Move Viewport to it's own class
	D3D12_VIEWPORT m_Viewport{};
	D3D12_RECT m_ViewportRect{};

	ComPtr<ID3D12DescriptorHeap> m_DepthHeap;
	ComPtr<ID3D12Resource> m_DepthStencil;
	DXGI_FORMAT m_DepthFormat{ DXGI_FORMAT_D32_FLOAT };
	// For SRV usage
	Descriptor m_DepthDescriptor;

	// Main Heap
	std::unique_ptr<DescriptorHeap> m_MainHeap;

	// 3rd-party allocator
	D3D12MA::Allocator* m_Allocator{ nullptr };

	// For ImGui
	ComPtr<ID3D12DescriptorHeap> m_guiAllocator;

public:

};
