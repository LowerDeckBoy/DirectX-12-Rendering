#include "Device.hpp"
#include "../Utils/Utils.hpp"
#include "Window.hpp"

Device::~Device()
{
	Release();
}

bool Device::Initialize(HWND hWnd)
{
	CreateDevice();
	CreateSwapChain(hWnd);
 
	return true;
}

void Device::CreateDevice()
{
	UINT dxgiFactoryFlags = 0;

#if defined (DEBUG) || (_DEBUG)
	//IID_PPV_ARGS
	ComPtr<ID3D12Debug1> debugController;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)), "Failed to create Debug Interface!");
	debugController->EnableDebugLayer();
	// Requires extra performence
	//debugController->SetEnableGPUBasedValidation(true);

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
	m_DebugDevice.Get()->ReportLiveDeviceObjects(D3D12_RLDO_IGNORE_INTERNAL);
#else
	m_DebugDevice.Get()->ReportLiveDeviceObjects(D3D12_RLDO_NONE);
#endif

	//for (uint32_t i = 0; i < Device::FrameCount; i++)
	//{
	//	ThrowIfFailed(m_Device.Get()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
	//				  IID_PPV_ARGS(m_CommandAllocators.at(i).GetAddressOf())));
	//}
	ThrowIfFailed(m_Device.Get()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
				  IID_PPV_ARGS(m_CommandAllocator.GetAddressOf())));

	m_Device.Get()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf()));

	ThrowIfFailed(m_CommandList.Get()->Close());
	
	CreateCommandQueue();
	CreateFences();

}

void Device::CreateSwapChain(HWND hWnd)
{
	assert(hWnd);

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
	ThrowIfFailed(m_Factory.Get()->CreateSwapChainForHwnd(m_CommandQueue.Get(), hWnd, &desc, &fullscreenDesc, nullptr, swapchain.GetAddressOf()), "Failed to create SwapChain!");

	ThrowIfFailed(m_Factory.Get()->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapchain.As(&m_SwapChain));
	m_CurrentBuffer = m_SwapChain.Get()->GetCurrentBackBufferIndex();

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
	}

	
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
	m_FenceValue = 1;

	m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(m_FenceEvent != nullptr);
}

void Device::ResizeBackBuffer()
{
	for (auto& buffer : m_RenderTargets)
		buffer->Release();


	m_DescriptorSize = m_Device.Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{ m_rtvHeap.Get()->GetCPUDescriptorHandleForHeapStart() };
	
	for (uint32_t i = 0; i < Device::FrameCount; i++)
	{
		ThrowIfFailed(m_SwapChain.Get()->GetBuffer(i, IID_PPV_ARGS(m_RenderTargets[i].GetAddressOf())));
		m_Device.Get()->CreateRenderTargetView(m_RenderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += static_cast<size_t>(1 * m_DescriptorSize);
	}
	
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
	m_Viewport.MinDepth = 0.1f;
	m_Viewport.MaxDepth = 1000.f;

}

void Device::Release()
{
	SafeRelease(m_CommandQueue);
	SafeRelease(m_CommandList);
	//for (auto& allocator : m_CommandAllocators)
	SafeRelease(m_CommandAllocator);

	//for (auto& fence : m_Fences)
	SafeRelease(m_Fence);

	for (auto& buffer : m_RenderTargets)
		SafeRelease(buffer);


	SafeRelease(m_rtvHeap);
	SafeRelease(m_SwapChain);
	SafeRelease(m_DebugDevice);
	SafeRelease(m_Device);
	SafeRelease(m_Factory);

	//OutputDebugStringA("Device destroyed.\n");
}
