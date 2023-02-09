#pragma once
#include <d3d12.h>
#include <d3dx12.h> // 3rd-party
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <cstdint>
#include <array>

// https://alain.xyz/blog/raw-directx12

class Window;

using Microsoft::WRL::ComPtr;

class Device
{
public:
	Device() = default;
	Device(const Device&) = delete;
	Device operator=(const Device&) = delete;
	~Device();

	static const uint32_t FrameCount{ 2 };

	bool Initialize(HWND hWnd);
	
	void CreateDevice();
	void CreateSwapChain(HWND hWnd);

	void CreateCommandList(ID3D12PipelineState* pPipelineState);
	void CreateCommandQueue();
	void CreateFences(D3D12_FENCE_FLAGS Flags = D3D12_FENCE_FLAG_NONE);
	
	void ResizeBackBuffer();
	void SetViewport();

	void Release();

public:
	// Getters
	IDXGIFactory4* GetFactory() const { return m_Factory.Get(); }
	ID3D12Device4* GetDevice() const { return m_Device.Get(); }
	IDXGISwapChain3* GetSwapChain() { return m_SwapChain.Get(); }

	ID3D12CommandAllocator* GetCommandAllocator() const { return m_CommandAllocator.Get(); }
	ID3D12CommandQueue* GetCommandQueue() const { return m_CommandQueue.Get(); }
	ID3D12GraphicsCommandList1* GetCommandList() { return m_CommandList.Get(); }

	//uint32_t GetFrameIndex() { return m_FrameIndex; }
	uint32_t GetFrameIndex() { return m_SwapChain.Get()->GetCurrentBackBufferIndex(); }
	void SetFrameIndex(uint32_t NewFrame) { m_FrameIndex = NewFrame; }
	ID3D12Fence1* GetFence() const { return m_Fence.Get(); }
	HANDLE& GetFenceEvent() { return m_FenceEvent; }

	ID3D12DescriptorHeap* GetDescriptorHeap() const { return m_rtvHeap.Get(); }
	uint32_t GetDescriptorSize() const { return m_DescriptorSize; }

	ID3D12Resource* GetRenderTargets() { return m_RenderTargets->Get(); }

	uint32_t m_CurrentBuffer{};

public:
	// Device
	ComPtr<IDXGIFactory4> m_Factory;

	ComPtr<ID3D12Device4> m_Device;
	ComPtr<ID3D12DebugDevice> m_DebugDevice;

	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12GraphicsCommandList1> m_CommandList;

	ComPtr<ID3D12RootSignature> m_RootSignature;
	//ComPtr<ID3D12PipelineState> m_PipelineState;

	// SwapChain
	ComPtr<IDXGISwapChain3> m_SwapChain;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12Resource> m_RenderTargets[FrameCount];
	uint32_t m_DescriptorSize{ 0 };

	uint32_t m_FrameIndex{ 0 };
	ComPtr<ID3D12Fence1> m_Fence;
	HANDLE m_FenceEvent;
	uint64_t m_FenceValue{ 1 };

	D3D12_VIEWPORT m_Viewport{};
	D3D12_RECT m_ViewportRect{};

};
