#include "Device.hpp"
#include "../Utils/Utils.hpp"
#include "Window.hpp"
#include <dxgidebug.h>

Device::~Device()
{
	Release();
}

bool Device::Initialize()
{
	CreateDevice();
	CreateCommandQueue();
	CreateFences();
	CreateSwapChain();
	CreateBackbuffer();
	CreateDescriptorHeaps();



	return true;
}

void Device::CreateDevice()
{
	UINT dxgiFactoryFlags = 0;

#if defined (DEBUG) || (_DEBUG)
	ComPtr<ID3D12Debug1> debugController;

	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)), "Failed to create Debug Interface!");
	debugController.Get()->EnableDebugLayer();
	debugController.Get()->SetEnableSynchronizedCommandQueueValidation(TRUE);

	debugController.Get()->SetEnableGPUBasedValidation(TRUE);

	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

	SafeRelease(debugController);

#endif

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(m_Factory.GetAddressOf())));

	ComPtr<IDXGIAdapter1> adapter;
	for (uint32_t i = 0; i < (uint32_t)m_Factory->EnumAdapters1(i, &adapter); i++)
	{
		DXGI_ADAPTER_DESC1 desc{};
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
			break;

		SafeRelease(adapter);
	}

	if (!adapter)
		throw std::exception();

	ComPtr<ID3D12Device> device;
	ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(device.GetAddressOf())),
				  "Failed to create D3D Device!");

	ThrowIfFailed(device.As(&m_Device));

	ThrowIfFailed(m_Device.Get()->QueryInterface(m_DebugDevice.GetAddressOf()));

#if defined (_DEBUG) | (DEBUG)
	//m_DebugDevice.Get()->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);

	ComPtr<IDXGIDebug1> dxgiDebug;
	ThrowIfFailed(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)));
	dxgiDebug.Get()->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
	//dxgiDebug->EnableLeakTrackingForThread();

	SafeRelease(dxgiDebug);
#else
	m_DebugDevice.Get()->ReportLiveDeviceObjects(D3D12_RLDO_NONE);
#endif

	for (uint32_t i = 0; i < FrameCount; i++)
	{
		ThrowIfFailed(m_Device.Get()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
					  IID_PPV_ARGS(m_CommandAllocators[i].GetAddressOf())));
		std::wstring nm{ L"Alloc" + std::to_wstring(i) };
		LPCWSTR name{ nm.c_str() };
		m_CommandAllocators[i]->SetName(name);
	}


	// Allocator
	D3D12MA::ALLOCATOR_DESC allocatorDesc{};
	allocatorDesc.pDevice = m_Device.Get();
	allocatorDesc.pAdapter = adapter.Get();
	D3D12MA::CreateAllocator(&allocatorDesc, &m_Allocator);

}

void Device::CreateSwapChain()
{
	SetViewport();

	DXGI_SWAP_CHAIN_DESC1 desc{};
	//desc.Flags = DXGI_SWAP_CHAIN_FLAG_DISPLAY_ONLY;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.BufferCount = Device::FrameCount;
	desc.Width = static_cast<uint32_t>(m_Viewport.Width);
	desc.Height = static_cast<uint32_t>(m_Viewport.Height);
	desc.SampleDesc.Count = 1;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc{};
	fullscreenDesc.Windowed = true;
	fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	fullscreenDesc.RefreshRate.Numerator = 0;
	fullscreenDesc.RefreshRate.Denominator = 0;

	ComPtr<IDXGISwapChain1> swapchain;
	ThrowIfFailed(m_Factory.Get()->CreateSwapChainForHwnd(m_CommandQueue.Get(), Window::GetHWND(), &desc, &fullscreenDesc, nullptr, swapchain.GetAddressOf()), "Failed to create SwapChain!");

	//swapchain.Get()->SetFullscreenState(TRUE, NULL);
	
	ThrowIfFailed(m_Factory.Get()->MakeWindowAssociation(Window::GetHWND(), DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapchain.As(&m_SwapChain));

	m_FrameIndex = m_SwapChain.Get()->GetCurrentBackBufferIndex();
	//m_SwapChain.Get()->SetMaximumFrameLatency(FrameCount);
	//m_SwapChain.Get()->GetFrameLatencyWaitableObject();
}

void Device::CreateBackbuffer()
{
	// RTV HEAP
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NumDescriptors = Device::FrameCount;

	ThrowIfFailed(m_Device.Get()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf())),
				  "Failed to create Descriptor Heap!");

	m_DescriptorSize = m_Device.Get()->GetDescriptorHandleIncrementSize(heapDesc.Type);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap.Get()->GetCPUDescriptorHandleForHeapStart());

	for (uint32_t i = 0; i < Device::FrameCount; i++)
	{
		ThrowIfFailed(m_SwapChain.Get()->GetBuffer(i, IID_PPV_ARGS(m_RenderTargets[i].GetAddressOf())));
		m_Device.Get()->CreateRenderTargetView(m_RenderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_DescriptorSize);
		std::wstring nm{ L"RTV" + std::to_wstring(i) };
		LPCWSTR name{ nm.c_str() };
		m_RenderTargets[i]->SetName(name);
	}
}

void Device::CreateDescriptorHeaps()
{
	// SRV HEAP
	D3D12_DESCRIPTOR_HEAP_DESC srvDesc{};
	srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvDesc.NumDescriptors = 1;

	ThrowIfFailed(m_Device.Get()->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(m_srvHeap.GetAddressOf())));
	m_srvHeap->SetName(L"SRV_HEAP");
	ThrowIfFailed(m_Device.Get()->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(m_guiAllocator.GetAddressOf())));
	m_guiAllocator->SetName(L"GUI_HEAP");

	srvDesc.NumDescriptors = 512;
	ThrowIfFailed(m_Device.Get()->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(m_cbvHeap.GetAddressOf())));
	m_cbvIncrementSize = m_Device.Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_cbvHandle = m_cbvHeap.Get()->GetCPUDescriptorHandleForHeapStart();
	m_cbvGpuHandle = m_cbvHeap.Get()->GetGPUDescriptorHandleForHeapStart();
	m_cbvHeap->SetName(L"CBV_HEAP");

	//D3D12MA::Pool* m_cbvPool{ nullptr };
	
	//D3D12MA::POOL_DESC poolDesc{};
	//poolDesc.Flags = D3D12MA::POOL_FLAG_NONE;

	ThrowIfFailed(m_Device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(m_cbvDescriptorHeap.m_Heap.GetAddressOf())));
	m_cbvDescriptorHeap.m_DescriptorSize = m_Device.Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_cbvDescriptorHeap.m_Heap->SetName(L"Main CBV Heap");
	m_cbvDescriptorHeap.m_NumDescriptors = srvDesc.NumDescriptors;

	// Sampler
	D3D12_DESCRIPTOR_HEAP_DESC samplerDesc{};
	samplerDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	samplerDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	samplerDesc.NumDescriptors = 64;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&samplerDesc, IID_PPV_ARGS(m_SamplerHeap.GetAddressOf())));
	m_SamplerHeap.Get()->SetName(L"Sampler Heap");


}

void Device::CreateCommandList(ID3D12PipelineState* pPipelineState)
{
	//pPipelineState
	m_Device.Get()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[m_FrameIndex].Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf()));
}

void Device::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC desc{};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	ThrowIfFailed(m_Device.Get()->CreateCommandQueue(&desc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())), "Failed to create Command Queue!\n");
}

void Device::CreateFences(D3D12_FENCE_FLAGS Flags)
{
	ThrowIfFailed(m_Device.Get()->CreateFence(0, Flags, IID_PPV_ARGS(m_Fence.GetAddressOf())));
	m_FenceValues[m_FrameIndex]++;

	m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(m_FenceEvent != nullptr);
}

void Device::SetViewport()
{
	m_ViewportRect.left = 0;
	m_ViewportRect.top = 0;
	m_ViewportRect.right = static_cast<uint32_t>(Window::GetDisplay().Width);
	m_ViewportRect.bottom = static_cast<uint32_t>(Window::GetDisplay().Height);

	m_Viewport.TopLeftX = 0.0f;
	m_Viewport.TopLeftY = 0.0f;
	m_Viewport.Width = static_cast<float>(Window::GetDisplay().Width);
	m_Viewport.Height = static_cast<float>(Window::GetDisplay().Height);
	//m_Viewport.MinDepth = 0.1f;
	//m_Viewport.MaxDepth = 1000.f;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;
}

void Device::Release()
{
	//SafeRelease(m_PipelineState);
	SafeRelease(m_RootSignature);
	SafeRelease(m_CommandQueue);
	SafeRelease(m_CommandList);

	for (auto& allocator : m_CommandAllocators)
		SafeRelease(allocator);

	SafeRelease(m_Fence);

	for (auto& buffer : m_RenderTargets)
		SafeRelease(buffer);

	SafeRelease(m_srvHeap);
	SafeRelease(m_rtvHeap);
	SafeRelease(m_SwapChain);

	m_Allocator->Release();

	SafeRelease(m_DebugDevice);
	SafeRelease(m_Device);
	SafeRelease(m_Factory);

	CloseHandle(m_FenceEvent);
}
