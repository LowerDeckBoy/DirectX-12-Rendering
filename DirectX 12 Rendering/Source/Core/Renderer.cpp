#include "Renderer.hpp"
#include "../Utils/Utils.hpp"
#include <array>

Renderer::Renderer(HINSTANCE hInstance)
{
	m_Window = std::make_unique<Window>(hInstance);
}

Renderer::~Renderer()
{
	
}

void Renderer::Initialize()
{
	m_Window->Initialize();
	auto hwnd = m_Window->GetHWND();
	Device::Initialize(hwnd);

}

void Renderer::Update()
{
}

void Renderer::Draw()
{
	//https://www.braynzarsoft.net/viewtutorial/q16390-03-initializing-directx-12

	RecordCommandLists();

	ComPtr<ID3D12CommandList> commandLists{ GetCommandList() };
	m_CommandQueue.Get()->ExecuteCommandLists(1, commandLists.GetAddressOf());

	ThrowIfFailed(GetSwapChain()->Present(1, 0));

	WaitForPreviousFrame();
}

void Renderer::RecordCommandLists()
{

	ThrowIfFailed(m_CommandAllocator.Get()->Reset());
	ThrowIfFailed(m_CommandList.Get()->Reset(m_CommandAllocator.Get(), nullptr));

	m_CommandList.Get()->RSSetViewports(1, &m_Viewport);
	m_CommandList.Get()->RSSetScissorRects(1, &m_ViewportRect);

	auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(),
														 D3D12_RESOURCE_STATE_PRESENT,
														 D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_CommandList.Get()->ResourceBarrier(1, &barrier1);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap.Get()->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, GetDescriptorSize());
	m_CommandList.Get()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	std::array<const float, 4> clearColor{ 0.5f, 0.5f, 1.0f, 1.0f };
	m_CommandList.Get()->ClearRenderTargetView(rtvHandle, clearColor.data(), 0, nullptr);

	auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(),
														 D3D12_RESOURCE_STATE_RENDER_TARGET,
														 D3D12_RESOURCE_STATE_PRESENT);
	m_CommandList.Get()->ResourceBarrier(1, &barrier2);

	ThrowIfFailed(m_CommandList.Get()->Close());
}

void Renderer::WaitForPreviousFrame()
{
	const uint64_t currentValue = m_FenceValue;
	ThrowIfFailed(GetCommandQueue()->Signal(GetFence(), currentValue));
	m_FenceValue++;

	if (GetFence()->GetCompletedValue() < currentValue)
	{
		ThrowIfFailed(GetFence()->SetEventOnCompletion(currentValue, m_FenceEvent));

		WaitForSingleObject(m_FenceEvent, INFINITE);
	}

	m_FrameIndex = GetSwapChain()->GetCurrentBackBufferIndex();
}
