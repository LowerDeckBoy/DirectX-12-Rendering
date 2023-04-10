#include "Renderer.hpp"
#include "../Utils/Utils.hpp"
#include "../Core/Window.hpp"
#include "../Rendering/Camera.hpp"
#include "GraphicsPipelineState.hpp"
#include <array>
#include <DirectXTex.h>


Renderer::~Renderer()
{
	OnDestroy();
}

void Renderer::Initialize(Camera& refCamera)
{
	m_GUI = std::make_unique<GUI>();
	m_Device = std::make_unique<Device>();
	m_Device->Initialize();
	m_GUI->Initialize(m_Device.get(), refCamera);

	CreateDepthStencil();
	InitModelPipeline();
	m_Device->CreateCommandList(m_ModelPipelineState.Get());

	LoadAssets();

	// Upload assets to GPU
	ThrowIfFailed(m_Device->GetCommandList()->Close());
	ID3D12CommandList* ppCommandLists[] = { m_Device->GetCommandList() };
	m_Device->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	
	WaitForGPU();
}

void Renderer::LoadAssets()
{
	//m_Model.Create(m_Device.get(), "Assets/glTF/sponza/scene.gltf");
	//m_Model.Create(m_Device.get(), "Assets/glTF/resto_ni_teo/scene.gltf");
	//m_Model.Create(m_Device.get(), "Assets/glTF/ice_cream_man/scene.gltf");
	//m_Model.Create(m_Device.get(), "Assets/glTF/lizard_mage/scene.gltf");
	//m_Model.Create(m_Device.get(), "Assets/glTF/pizza_ballerina/scene.gltf");
	//m_Model.Create(m_Device.get(), "Assets/glTF/realistic_armor/scene.gltf");
	//m_Model.Create(m_Device.get(), "Assets/glTF/smart_littel_robot/scene.gltf");
	//m_Model.Create(m_Device.get(), "Assets/glTF/sphere/scene.gltf");
	//m_Model.Create(m_Device.get(), "Assets/glTF/mathilda/scene.gltf");
	//m_Model.Create(m_Device.get(), "Assets/glTF/fallout_ranger/scene.gltf");
	//m_Model.Create(m_Device.get(), "Assets/glTF/globophobia/scene.gltf");
	m_Model.Create(m_Device.get(), "Assets/glTF/damaged_helmet/scene.gltf");
	//m_Model.Create(m_Device.get(), "Assets/fbx/sphere/source/Perfect360.fbx");
	//m_Model.Create(m_Device.get(), "Assets/fbx/bull/source/Bull_Lowpoly.fbx");
	//m_Model.Create(m_Device.get(), "Assets/glTF/anemone_deer/scene.gltf");
	// With anims
	//m_Model.Create(m_Device.get(), "Assets/glTF/Nowy sgd162_idle_walk_cycle/scene.gltf");

	// Skybox
	m_Skybox.Create(m_Device.get());
}

void Renderer::Update(XMMATRIX ViewProj)
{

}

void Renderer::Draw(Camera* pCamera)
{
	RecordCommandLists(m_Device->m_FrameIndex, pCamera);

	ID3D12CommandList* commandLists[]{ m_Device->GetCommandList() };
	m_Device->GetCommandQueue()->ExecuteCommandLists(_countof(commandLists), commandLists);

	HRESULT hResult{ m_Device->GetSwapChain()->Present(1, 0) };
	if (hResult == DXGI_ERROR_DEVICE_REMOVED || hResult == DXGI_ERROR_DEVICE_RESET)
	{
		::MessageBox(Window::GetHWND(), L"Device removed or Device reset", L"DXGI Error", MB_OK);
		throw std::exception();
	}

	MoveToNextFrame();
}

void Renderer::RecordCommandLists(uint32_t CurrentFrame, Camera* pCamera)
{
	ThrowIfFailed(m_Device->m_CommandAllocators[m_Device->m_FrameIndex]->Reset());

	ThrowIfFailed(m_Device->GetCommandList()->Reset(m_Device->m_CommandAllocators[m_Device->m_FrameIndex].Get(), 
													m_ModelPipelineState.Get()));

	m_GUI->Begin();
	m_Device->GetCommandList()->SetPipelineState(m_ModelPipelineState.Get());
	m_Device->GetCommandList()->SetGraphicsRootSignature(m_ModelRootSignature.Get());
	m_Device->GetCommandList()->RSSetViewports(1, &m_Device->m_Viewport);
	m_Device->GetCommandList()->RSSetScissorRects(1, &m_Device->m_ViewportRect);

	TransitToRender();

	ID3D12DescriptorHeap* ppHeaps[] = { m_Device->m_cbvDescriptorHeap.GetHeap() };
	m_Device->GetCommandList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// RTV and Depth
	SetRenderTarget();
	
	m_Model.Draw(pCamera);
	m_Model.DrawGUI();

	m_Device->GetCommandList()->SetPipelineState(m_SkyboxPipelineState.Get());
	m_Device->GetCommandList()->SetGraphicsRootSignature(m_SkyboxRootSignature.Get());
	m_Skybox.Draw(pCamera);
	m_GUI->End(m_Device->GetCommandList());

	TransitToPresent();

	ThrowIfFailed(m_Device->GetCommandList()->Close());
}

void Renderer::OnResize()
{
	WaitForGPU();
	FlushGPU();
	ResizeBackbuffers();

	WaitForGPU();
}

void Renderer::ResizeBackbuffers()
{
	if (!m_Device->GetDevice() || !m_Device->GetSwapChain() || !m_Device->m_CommandAllocators[m_Device->m_FrameIndex])
		throw std::exception();

	m_Device->m_CommandAllocators[m_Device->m_FrameIndex]->Reset();
	m_Device->GetCommandList()->Reset(m_Device->m_CommandAllocators[m_Device->m_FrameIndex].Get(), m_ModelPipelineState.Get());

	//m_Device->GetCommandList()->OMSetRenderTargets(0, nullptr, FALSE, nullptr);

	for (uint32_t i = 0; i < Device::FrameCount; i++)
	{
		m_Device->m_RenderTargets[i]->Release();
	}

	m_DepthStencil->Release();

	HRESULT hResult{ m_Device->GetSwapChain()->ResizeBuffers(Device::FrameCount,
											static_cast<uint32_t>(Window::GetDisplay().Width),
											static_cast<uint32_t>(Window::GetDisplay().Height),
											DXGI_FORMAT_R8G8B8A8_UNORM, 0) };
	if (hResult == DXGI_ERROR_DEVICE_REMOVED || hResult == DXGI_ERROR_DEVICE_RESET)
	{
		OutputDebugStringA("Device removed!\n");
	}

	m_Device->m_FrameIndex = 0;

	m_Device->SetViewport();
	m_Device->CreateBackbuffer();
	CreateDepthStencil();

	m_Device->m_CommandList->Close();
	ID3D12CommandList* ppCmdLists[]{ m_Device->m_CommandList.Get() };
	m_Device->m_CommandQueue.Get()->ExecuteCommandLists(_countof(ppCmdLists), ppCmdLists);

	WaitForGPU();
}

void Renderer::FlushGPU()
{
	for (uint32_t i = 0; i < Device::FrameCount; i++)
	{
		const uint64_t currentValue = m_Device->m_FenceValues[i];

		ThrowIfFailed(m_Device->GetCommandQueue()->Signal(m_Device->GetFence(), currentValue));
		m_Device->m_FenceValues[i]++;

		if (m_Device->GetFence()->GetCompletedValue() < currentValue)
		{
			ThrowIfFailed(m_Device->GetFence()->SetEventOnCompletion(currentValue, m_Device->m_FenceEvent));

			WaitForSingleObject(m_Device->m_FenceEvent, INFINITE);
		}
	}

	m_Device->m_FrameIndex = 0;
}

void Renderer::MoveToNextFrame()
{
	const UINT64 currentFenceValue = m_Device->m_FenceValues[m_Device->m_FrameIndex];
	ThrowIfFailed(m_Device->m_CommandQueue->Signal(m_Device->m_Fence.Get(), currentFenceValue));

	// Update the frame index.
	m_Device->m_FrameIndex = m_Device->m_SwapChain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (m_Device->m_Fence->GetCompletedValue() < m_Device->m_FenceValues[m_Device->m_FrameIndex])
	{
		ThrowIfFailed(m_Device->m_Fence->SetEventOnCompletion(m_Device->m_FenceValues[m_Device->m_FrameIndex], m_Device->m_FenceEvent));
		WaitForSingleObjectEx(m_Device->m_FenceEvent, INFINITE, FALSE);
	}

	m_Device->m_FenceValues[m_Device->m_FrameIndex] = currentFenceValue + 1;
}

void Renderer::WaitForGPU()
{
	ThrowIfFailed(m_Device->GetCommandQueue()->Signal(m_Device->GetFence(), m_Device->m_FenceValues[m_Device->m_FrameIndex]));

	ThrowIfFailed(m_Device->m_Fence->SetEventOnCompletion(m_Device->m_FenceValues[m_Device->m_FrameIndex], m_Device->m_FenceEvent));
	::WaitForSingleObjectEx(m_Device->m_FenceEvent, INFINITE, FALSE);

	m_Device->m_FenceValues[m_Device->m_FrameIndex]++;
}

void Renderer::SetRenderTarget()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_Device->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(), m_Device->m_FrameIndex, m_Device->GetDescriptorSize());
	CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle(m_DepthHeap.Get()->GetCPUDescriptorHandleForHeapStart());
	m_Device->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &depthHandle);

	ClearRenderTarget(rtvHandle, depthHandle);
}

void Renderer::ClearRenderTarget(CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle)
{
	m_Device->GetCommandList()->ClearRenderTargetView(rtvHandle, m_ClearColor.data(), 0, nullptr);
	m_Device->GetCommandList()->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Renderer::TransitToRender()
{
	auto presentToRender = CD3DX12_RESOURCE_BARRIER::Transition(m_Device->m_RenderTargets[m_Device->m_FrameIndex].Get(),
																D3D12_RESOURCE_STATE_PRESENT,
																D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_Device->GetCommandList()->ResourceBarrier(1, &presentToRender);
}

void Renderer::TransitToPresent()
{
	auto renderToPresent = CD3DX12_RESOURCE_BARRIER::Transition(m_Device->m_RenderTargets[m_Device->m_FrameIndex].Get(),
																D3D12_RESOURCE_STATE_RENDER_TARGET,
																D3D12_RESOURCE_STATE_PRESENT);
	m_Device->GetCommandList()->ResourceBarrier(1, &renderToPresent);
}

void Renderer::CreateDepthStencil()
{
	D3D12_DESCRIPTOR_HEAP_DESC dsHeap{};
	dsHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsHeap.NumDescriptors = 1;
	ThrowIfFailed(m_Device->GetDevice()->CreateDescriptorHeap(&dsHeap, IID_PPV_ARGS(m_DepthHeap.GetAddressOf())));


	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
	auto heapDesc{ CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT,
												static_cast<uint64_t>(m_Device->GetViewport().Width),
												static_cast<uint32_t>(m_Device->GetViewport().Height),
												1, 0, 1, 0,
												D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) };

	ThrowIfFailed(m_Device->GetDevice()->CreateCommittedResource(&heapProperties,
				  D3D12_HEAP_FLAG_NONE,
				  &heapDesc,
				  D3D12_RESOURCE_STATE_DEPTH_WRITE,
				  &clearValue,
				  IID_PPV_ARGS(m_DepthStencil.GetAddressOf())));
	m_DepthHeap.Get()->SetName(L"Depth Heap");

	D3D12_DEPTH_STENCIL_VIEW_DESC dsView{};
	dsView.Flags = D3D12_DSV_FLAG_NONE;
	dsView.Format = DXGI_FORMAT_D32_FLOAT;
	dsView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsView.Texture2D.MipSlice = 0;

	m_Device->GetDevice()->CreateDepthStencilView(m_DepthStencil.Get(), &dsView, m_DepthHeap.Get()->GetCPUDescriptorHandleForHeapStart());
	m_DepthStencil.Get()->SetName(L"Depth Stencil");
}

void Renderer::InitModelPipeline()
{
	//m_VertexShader->Create(L"Assets/Shaders/VS_GLTF.hlsl");
	m_VertexShader->Create("Assets/Shaders/VS_GLTF.hlsl", "VS", "vs_5_1");
	m_PixelShader->Create("Assets/Shaders/Normal_Mapping_Pixel.hlsl", "PS", "ps_5_1");
	//m_PixelShader->Create("Assets/Shaders/Specular_Mapping_Pixel.hlsl", "PS", "ps_5_1");
	//m_PixelShader->Create( L"Assets/Shaders/PS_GLTF.hlsl");
	//m_PixelShader->Create( L"Assets/Shaders/Outline_Pixel.hlsl");
	
	auto layout{ GraphicsPipelineState::CreateInputLayout() };

	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{ D3D_ROOT_SIGNATURE_VERSION_1_1 };
	if (FAILED(m_Device->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		// Rollback to 1_0
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	// TODO: move ranges and params into GraphicsPipelineState class
	std::array<CD3DX12_DESCRIPTOR_RANGE1, 7> ranges{};
	// Vertex Constant Buffer
	ranges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	ranges.at(1).Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	// Pixel Constant Buffer
	ranges.at(2).Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	// Textures
	ranges.at(3).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	ranges.at(4).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	ranges.at(5).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	ranges.at(6).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	
	std::array<CD3DX12_ROOT_PARAMETER1, 7> params{};
	params.at(0).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	params.at(1).InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	params.at(2).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	params.at(3).InitAsDescriptorTable(1, &ranges.at(3), D3D12_SHADER_VISIBILITY_PIXEL);
	params.at(4).InitAsDescriptorTable(1, &ranges.at(4), D3D12_SHADER_VISIBILITY_PIXEL);
	params.at(5).InitAsDescriptorTable(1, &ranges.at(5), D3D12_SHADER_VISIBILITY_PIXEL);
	params.at(6).InitAsDescriptorTable(1, &ranges.at(6), D3D12_SHADER_VISIBILITY_PIXEL);

	auto rootFlags{ GraphicsPipelineState::SetRootFlags() };
	auto staticSampler{ GraphicsPipelineState::CreateStaticSampler(0) };
	
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootDesc{};
	rootDesc.Init_1_1(static_cast<uint32_t>(params.size()), params.data(), 1, &staticSampler, rootFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;

	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, signature.GetAddressOf(), error.GetAddressOf()));
	
	ThrowIfFailed(m_Device->GetDevice()->CreateRootSignature(0,
				  signature.Get()->GetBufferPointer(), signature.Get()->GetBufferSize(),
				  IID_PPV_ARGS(m_ModelRootSignature.GetAddressOf())));
	m_ModelRootSignature.Get()->SetName(L"ModelRootSignature");

	auto pipelineStateDesc{ GraphicsPipelineState::CreateState(m_ModelRootSignature.Get(), m_VertexShader.get(), m_PixelShader.get(), layout) };

	ThrowIfFailed(m_Device->GetDevice()->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(m_ModelPipelineState.GetAddressOf())));
	m_ModelPipelineState.Get()->SetName(L"ModelPipelineState");

	// Skybox pipeline
	// TODO: Move inside Skybox class
	auto skyboxLayout{ GraphicsPipelineState::CreateSkyboxInputLayout() };
	m_SkyboxVS->Create("Assets/Shaders/Skybox_VS.hlsl", "VS", "vs_5_1");
	m_SkyboxPS->Create("Assets/Shaders/Skybox_PS.hlsl", "PS", "ps_5_1");

	ThrowIfFailed(m_Device->GetDevice()->CreateRootSignature(0,
		signature.Get()->GetBufferPointer(), signature.Get()->GetBufferSize(),
		IID_PPV_ARGS(m_SkyboxRootSignature.GetAddressOf())));
	m_ModelRootSignature.Get()->SetName(L"SkyboxRootSignature");

	pipelineStateDesc.InputLayout = { skyboxLayout.data(), static_cast<uint32_t>(skyboxLayout.size()) };
	pipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(m_SkyboxVS->GetData());
	pipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(m_SkyboxPS->GetData());
	ThrowIfFailed(m_Device->GetDevice()->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(m_SkyboxPipelineState.GetAddressOf())));
	m_SkyboxPipelineState.Get()->SetName(L"SkyboxPipelineState");
	
}

void Renderer::OnDestroy()
{
	WaitForGPU();

	::CloseHandle(m_Device->m_FenceEvent);
}
