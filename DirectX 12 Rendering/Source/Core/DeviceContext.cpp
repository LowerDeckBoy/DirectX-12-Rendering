#include "DeviceContext.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Utilities/Logger.hpp"
#include "Window.hpp"

#if defined (_DEBUG) || (DEBUG)
#include <dxgidebug.h>
#endif

uint32_t DeviceContext::FRAME_INDEX = 0;

DeviceContext::~DeviceContext()
{
	Release();
}

bool DeviceContext::Initialize()
{
	CreateDevice();
	CreateCommandQueue();
	CreateCommandList();
	CreateFences();
	CreateSwapChain();
	CreateDescriptorHeaps();
	CreateBackbuffer();
	//CreateDepthStencil();

	return true;
}

void DeviceContext::CreateDevice()
{
	UINT dxgiFactoryFlags = 0;

#if defined (_DEBUG) || (DEBUG)
	ComPtr<ID3D12Debug1> debugController;

	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)), "Failed to create Debug Interface!");
	debugController.Get()->EnableDebugLayer();
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	SAFE_RELEASE(debugController);

	ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
	ThrowIfFailed(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf())));
	dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
	dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);

#endif

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(m_Factory.GetAddressOf())));

	ComPtr<IDXGIAdapter1> adapter;
	for (uint32_t i = 0; i < static_cast<uint32_t>(m_Factory->EnumAdapters1(i, &adapter)); i++)
	{
		DXGI_ADAPTER_DESC1 desc{};
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
		{
			SAFE_RELEASE(adapter);
			break;
		}
	}

	if (!adapter)
		throw std::exception();

	// Get some debug data here
	//DXGI_ADAPTER_DESC1 adapterInfo{};
	//adapter->GetDesc1(&adapterInfo);
	//adapterInfo.

	ComPtr<ID3D12Device> device;
	ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(device.GetAddressOf())),
				  "Failed to create D3D Device!");

	ThrowIfFailed(device.As(&m_Device));

	ThrowIfFailed(m_Device.Get()->QueryInterface(m_DebugDevice.GetAddressOf()));

#if defined (_DEBUG) | (DEBUG)

	ComPtr<IDXGIDebug1> dxgiDebug;
	ThrowIfFailed(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)));
	dxgiDebug.Get()->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
	//dxgiDebug->EnableLeakTrackingForThread();

	SAFE_RELEASE(dxgiDebug);
#else
	m_DebugDevice.Get()->ReportLiveDeviceObjects(D3D12_RLDO_NONE);
#endif

	for (uint32_t i = 0; i < FRAME_COUNT; i++)
	{
		ThrowIfFailed(m_Device.Get()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
					  IID_PPV_ARGS(m_CommandAllocators.at(i).GetAddressOf())));

		std::wstring nm{ L"Alloc" + std::to_wstring(i) };
		LPCWSTR name{ nm.c_str() };
		m_CommandAllocators.at(i)->SetName(name);
	}

	// Allocator
	D3D12MA::ALLOCATOR_DESC allocatorDesc{};
	allocatorDesc.pDevice = m_Device.Get();
	allocatorDesc.pAdapter = adapter.Get();
	D3D12MA::CreateAllocator(&allocatorDesc, m_Allocator.ReleaseAndGetAddressOf());

}

void DeviceContext::CreateSwapChain()
{
	SetViewport();

	DXGI_SWAP_CHAIN_DESC1 desc{};
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.BufferCount = DeviceContext::FRAME_COUNT;
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

	FRAME_INDEX = m_SwapChain.Get()->GetCurrentBackBufferIndex();
}

void DeviceContext::CreateBackbuffer()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NumDescriptors = DeviceContext::FRAME_COUNT;

	ThrowIfFailed(m_Device.Get()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_RenderTargetHeap.GetAddressOf())),
				  "Failed to create Descriptor Heap!");

	m_DescriptorSize = m_Device.Get()->GetDescriptorHandleIncrementSize(heapDesc.Type);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RenderTargetHeap.Get()->GetCPUDescriptorHandleForHeapStart());
	for (uint32_t i = 0; i < DeviceContext::FRAME_COUNT; i++)
	{
		ThrowIfFailed(m_SwapChain.Get()->GetBuffer(i, IID_PPV_ARGS(m_RenderTargets.at(i).GetAddressOf())));
		m_Device.Get()->CreateRenderTargetView(m_RenderTargets.at(i).Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_DescriptorSize);

		std::wstring nm{ L"RTV" + std::to_wstring(i) };
		LPCWSTR name{ nm.c_str() };
		m_RenderTargets.at(i)->SetName(name);
	}
}

DXGI_FORMAT DeviceContext::GetRTVFormat() const noexcept
{
	return m_RenderTargetFormat;
}

void DeviceContext::CreateDescriptorHeaps()
{
	// SRV HEAP
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = FRAME_COUNT;

	ThrowIfFailed(m_Device.Get()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_guiAllocator.GetAddressOf())));
	m_guiAllocator->SetName(L"GUI_HEAP");

	desc.NumDescriptors = 4096;
	m_MainHeap = std::make_unique<DescriptorHeap>();
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_MainHeap->GetHeapAddressOf())));
	m_MainHeap->SetDescriptorSize(m_Device.Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	m_MainHeap->GetHeap()->SetName(L"Main Descriptor Heap");
	m_MainHeap->SetDescriptorsCount(desc.NumDescriptors);

	// Depth Heap
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NumDescriptors = 1;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_DepthHeap.ReleaseAndGetAddressOf())));
}

void DeviceContext::CreateCommandList(ID3D12PipelineState* pPipelineState)
{
	m_Device.Get()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators.at(FRAME_INDEX).Get(), pPipelineState, IID_PPV_ARGS(m_CommandList.GetAddressOf()));
	m_CommandList.Get()->SetName(L"Command List");
}

void DeviceContext::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC desc{};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_Device.Get()->CreateCommandQueue(&desc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())), "Failed to create Command Queue!\n");
	m_CommandQueue->SetName(L"Direct Command Queue");

	desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	ThrowIfFailed(m_Device.Get()->CreateCommandQueue(&desc, IID_PPV_ARGS(m_ComputeQueue.GetAddressOf())), "Failed to create Compute Queue!\n");
	m_ComputeQueue->SetName(L"Compute Command Queue");

}

void DeviceContext::CreateFences(D3D12_FENCE_FLAGS Flags)
{
	ThrowIfFailed(m_Device.Get()->CreateFence(0, Flags, IID_PPV_ARGS(m_Fence.GetAddressOf())));
	m_FenceValues.at(FRAME_INDEX)++;

	m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(m_FenceEvent != nullptr);
}

void DeviceContext::CreateDepthStencil()
{
	D3D12_DESCRIPTOR_HEAP_DESC dsHeap{};
	dsHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsHeap.NumDescriptors = 1;
	ThrowIfFailed(m_Device.Get()->CreateDescriptorHeap(&dsHeap, IID_PPV_ARGS(m_DepthHeap.GetAddressOf())));
	m_DepthHeap.Get()->SetName(L"Depth Heap");

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = m_DepthFormat;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	const auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
	const auto heapDesc{ CD3DX12_RESOURCE_DESC::Tex2D(m_DepthFormat,
													static_cast<uint64_t>(m_Viewport.Width),
													static_cast<uint32_t>(m_Viewport.Height),
													1, 0, 1, 0,
													D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) };

	if (m_DepthStencil.Get())
		m_DepthStencil->Release();

	ThrowIfFailed(m_Device.Get()->CreateCommittedResource(&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&heapDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(m_DepthStencil.ReleaseAndGetAddressOf())));
	m_DepthStencil.Get()->SetName(L"Depth Stencil");

	D3D12_DEPTH_STENCIL_VIEW_DESC dsView{};
	dsView.Flags = D3D12_DSV_FLAG_NONE;
	dsView.Format = m_DepthFormat;
	dsView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsView.Texture2D.MipSlice = 0;

	m_Device.Get()->CreateDepthStencilView(m_DepthStencil.Get(), &dsView, m_DepthHeap.Get()->GetCPUDescriptorHandleForHeapStart());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	m_MainHeap->Allocate(m_DepthDescriptor);
	m_Device.Get()->CreateShaderResourceView(m_DepthStencil.Get(), &srvDesc, m_DepthDescriptor.GetCPU());

}

void DeviceContext::SetViewport()
{
	m_ViewportRect.left   = 0;
	m_ViewportRect.top	  = 0;
	m_ViewportRect.right  = static_cast<uint64_t>(Window::GetDisplay().Width);
	m_ViewportRect.bottom = static_cast<uint64_t>(Window::GetDisplay().Height);

	m_Viewport.TopLeftX = 0.0f;
	m_Viewport.TopLeftY = 0.0f;
	m_Viewport.Width	= static_cast<float>(Window::GetDisplay().Width);
	m_Viewport.Height	= static_cast<float>(Window::GetDisplay().Height);
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;
}

void DeviceContext::FlushGPU()
{
	for (uint32_t i = 0; i < DeviceContext::FRAME_COUNT; i++)
	{
		const uint64_t currentValue = m_FenceValues.at(i);

		ThrowIfFailed(m_CommandQueue->Signal(GetFence(), currentValue));
		m_FenceValues.at(i)++;

		if (GetFence()->GetCompletedValue() < currentValue)
		{
			ThrowIfFailed(GetFence()->SetEventOnCompletion(currentValue, m_FenceEvent));

			WaitForSingleObject(m_FenceEvent, INFINITE);
		}
	}

	FRAME_INDEX = 0;
}

void DeviceContext::MoveToNextFrame()
{
	const UINT64 currentFenceValue = m_FenceValues.at(FRAME_INDEX);
	ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), currentFenceValue));

	// Update the frame index.
	FRAME_INDEX = m_SwapChain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (m_Fence->GetCompletedValue() < m_FenceValues.at(FRAME_INDEX))
	{
		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValues.at(FRAME_INDEX), m_FenceEvent));
		WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
	}

	m_FenceValues.at(FRAME_INDEX) = currentFenceValue + 1;
}

void DeviceContext::WaitForGPU()
{
	ThrowIfFailed(GetCommandQueue()->Signal(GetFence(), m_FenceValues.at(FRAME_INDEX)));

	ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValues.at(FRAME_INDEX), m_FenceEvent));
	::WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

	m_FenceValues.at(FRAME_INDEX)++;
}

void DeviceContext::ExecuteCommandList()
{
	ThrowIfFailed(GetCommandList()->Close());
	std::array<ID3D12CommandList*, 1> ppCommandLists{ GetCommandList() };
	GetCommandQueue()->ExecuteCommandLists(static_cast<uint32_t>(ppCommandLists.size()), ppCommandLists.data());
	WaitForGPU();
}

void DeviceContext::OnResize()
{
	if (!m_Device.Get() || !m_SwapChain.Get() || !GetCommandAllocator())
		throw std::exception();

	GetCommandAllocator()->Reset();
	GetCommandList()->Reset(GetCommandAllocator(), nullptr);

	ReleaseRenderTargets();

	if (m_DepthStencil.Get())
		m_DepthStencil->Release();

	const HRESULT hResult{ m_SwapChain.Get()->ResizeBuffers(DeviceContext::FRAME_COUNT,
												static_cast<uint32_t>(Window::GetDisplay().Width),
												static_cast<uint32_t>(Window::GetDisplay().Height),
												DXGI_FORMAT_R8G8B8A8_UNORM, 0) };

	if (hResult == DXGI_ERROR_DEVICE_REMOVED || hResult == DXGI_ERROR_DEVICE_RESET)
	{
		::OutputDebugStringA("Device removed!\n");
		throw std::exception();
	}

	FRAME_INDEX = 0;

	SetViewport();
	CreateBackbuffer();
	CreateDepthStencil();

	ExecuteCommandList();
}

void DeviceContext::Release()
{
	SAFE_RELEASE(m_DepthStencil);
	SAFE_RELEASE(m_DepthHeap);

	SAFE_RELEASE(m_ComputeQueue);
	SAFE_RELEASE(m_CommandQueue);
	SAFE_RELEASE(m_CommandList);

	SAFE_RELEASE(m_guiAllocator);

	for (auto& allocator : m_CommandAllocators)
		SAFE_RELEASE(allocator);

	SAFE_RELEASE(m_Fence);

	for (auto& buffer : m_RenderTargets)
		SAFE_RELEASE(buffer);

	SAFE_RELEASE(m_RenderTargetHeap);
	SAFE_RELEASE(m_SwapChain);

	SAFE_RELEASE(m_Allocator);

	SAFE_RELEASE(m_DebugDevice);
	SAFE_RELEASE(m_Device);
	SAFE_RELEASE(m_Factory);

	Logger::Log("DeviceContext released.");
}

IDXGIFactory4* DeviceContext::GetFactory() const noexcept
{
	return m_Factory.Get();
}

ID3D12Device5* DeviceContext::GetDevice() noexcept
{
	return m_Device.Get();
}

IDXGISwapChain3* DeviceContext::GetSwapChain() noexcept
{
	return m_SwapChain.Get();
}

ID3D12CommandAllocator* DeviceContext::GetCommandAllocator() const
{
	return m_CommandAllocators.at(FRAME_INDEX).Get();
}

ID3D12CommandQueue* DeviceContext::GetCommandQueue() const noexcept
{
	return m_CommandQueue.Get();
}

ID3D12GraphicsCommandList4* DeviceContext::GetCommandList() noexcept
{
	return m_CommandList.Get();
}

ID3D12CommandQueue* DeviceContext::GetComputeQueue() const noexcept
{
	return m_ComputeQueue.Get();
}

uint32_t DeviceContext::GetFrameIndex()
{
	return m_SwapChain.Get()->GetCurrentBackBufferIndex();
}

ID3D12Fence1* DeviceContext::GetFence() const noexcept
{
	return m_Fence.Get();
}

HANDLE& DeviceContext::GetFenceEvent() noexcept
{
	return m_FenceEvent;
}

ID3D12Resource* DeviceContext::GetRenderTarget() const noexcept
{
	return m_RenderTargets.at(FRAME_INDEX).Get();
}

ID3D12DescriptorHeap* DeviceContext::GetRenderTargetHeap() const noexcept
{
	return m_RenderTargetHeap.Get();
}

uint32_t DeviceContext::GetDescriptorSize() const noexcept
{
	return m_DescriptorSize;
}

D3D12_VIEWPORT DeviceContext::GetViewport() const noexcept
{
	return m_Viewport;
}

D3D12_RECT DeviceContext::GetScissor() const noexcept
{
	return m_ViewportRect;
}

ID3D12DescriptorHeap* DeviceContext::GetDepthHeap() noexcept
{
	return m_DepthHeap.Get();
}

ID3D12Resource* DeviceContext::GetDepthStencil() const noexcept
{
	return m_DepthStencil.Get();
}

DXGI_FORMAT DeviceContext::GetDepthFormat() const noexcept
{
	return m_DepthFormat;
}

Descriptor DeviceContext::GetDepthDescriptor() const noexcept
{
	return m_DepthDescriptor;
}

void DeviceContext::ReleaseRenderTargets()
{
	for (auto& target : m_RenderTargets)
	{
		//target.Reset();
		target = nullptr;
		//target->Release();
	}
}

inline ID3D12DescriptorHeap* DeviceContext::GetGUIHeap() const noexcept
{
	return m_guiAllocator.Get();
}

DescriptorHeap* DeviceContext::GetMainHeap() noexcept
{
	return m_MainHeap.get();
}

D3D12MA::Allocator* DeviceContext::GetAllocator() const noexcept
{
	return m_Allocator.Get();
}
