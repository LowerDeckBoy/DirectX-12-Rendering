#pragma once
#include <d3d12.h>
#include <d3dx12.h> // 3rd-party
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <cstdint>
#include <array>


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

	bool Initialize();
	
	void CreateDevice();
	void CreateSwapChain();
	void CreateBackbuffer();

	inline DXGI_FORMAT GetRTVFormat() const { return m_rtvFormat; }

	void CreateDescriptorHeaps();
	void CreateCommandList(ID3D12PipelineState* pPipelineState);
	void CreateCommandQueue();
	void CreateFences(D3D12_FENCE_FLAGS Flags = D3D12_FENCE_FLAG_NONE);
	
	void SetViewport();

	void Release();

public:
	// Getters
	inline IDXGIFactory4* GetFactory() const { return m_Factory.Get(); }
	inline ID3D12Device4* GetDevice() const { return m_Device.Get(); }
	inline IDXGISwapChain3* GetSwapChain() { return m_SwapChain.Get(); }

	// Command getters
	//inline ID3D12CommandAllocator* GetCommandAllocator() const { return m_CommandAllocator.Get(); }
	inline ID3D12CommandQueue* GetCommandQueue() const { return m_CommandQueue.Get(); }
	inline ID3D12GraphicsCommandList1* GetCommandList() { return m_CommandList.Get(); }

	// Frame/Fence getters
	inline uint32_t GetFrameIndex() { return m_SwapChain.Get()->GetCurrentBackBufferIndex(); }
	void SetFrameIndex(uint32_t NewFrame) { m_FrameIndex = NewFrame; }
	inline ID3D12Fence1* GetFence() const { return m_Fence.Get(); }
	inline HANDLE& GetFenceEvent() { return m_FenceEvent; }

	// Heap getters
	inline ID3D12DescriptorHeap* GetDescriptorHeap() const { return m_rtvHeap.Get(); }
	inline uint32_t GetDescriptorSize() const { return m_DescriptorSize; }
	inline ID3D12DescriptorHeap* GetSRVHeap() const { return m_srvHeap.Get(); }
	inline ID3D12DescriptorHeap* GetCBVHeap() const { return m_cbvHeap.Get(); }

	inline ID3D12Resource* GetRenderTargets() { return m_RenderTargets->Get(); }
	inline D3D12_VIEWPORT& GetViewport() { return m_Viewport; }

	// TODO: Move staff to seprate classes 
	// for maintaining better accessability to resources
public:
	// Device
	ComPtr<IDXGIFactory4> m_Factory;

	ComPtr<ID3D12Device4> m_Device;
	ComPtr<ID3D12DebugDevice> m_DebugDevice;

	ComPtr<ID3D12CommandAllocator> m_CommandAllocators[FrameCount];
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12GraphicsCommandList1> m_CommandList;

	ComPtr<ID3D12RootSignature> m_RootSignature;

	// SwapChain
	ComPtr<IDXGISwapChain3> m_SwapChain;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	ComPtr<ID3D12Resource> m_RenderTargets[FrameCount];
	uint32_t m_DescriptorSize{ 0 };

	DXGI_FORMAT m_rtvFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };

	uint32_t m_FrameIndex{ 0 };
	ComPtr<ID3D12Fence1> m_Fence;
	HANDLE m_FenceEvent;
	uint64_t m_FenceValues[FrameCount]{};

	// TODO: Move Viewport to it's own class
	D3D12_VIEWPORT m_Viewport{};
	D3D12_RECT m_ViewportRect{};

	// For ImGui
	inline ID3D12DescriptorHeap* GetGUIHeap() const { return m_guiAllocator.Get(); }
	ComPtr<ID3D12DescriptorHeap> m_guiAllocator;

};
