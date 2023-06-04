#include "Renderer.hpp"
#include "../Utils/Utilities.hpp"
#include "../Core/Window.hpp"
#include "../Rendering/Camera.hpp"
#include "GraphicsPipelineState.hpp"
#include <array>

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

	//PreRender();
	//m_IBL.Test(m_Device.get());

	// Upload assets to GPU
	ThrowIfFailed(m_Device->GetCommandList()->Close());
	ID3D12CommandList* ppCommandLists[] = { m_Device->GetCommandList() };
	m_Device->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_Device->WaitForGPU();
}

void Renderer::LoadAssets()
{
	m_Skybox = new Skybox(m_Device.get());

	m_Model = std::make_unique<Model>(m_Device.get(), "Assets/glTF/sponza/Sponza.gltf");
	//m_Model = std::make_unique<Model>(m_Device.get(), "Assets/glTF/damaged_helmet/scene.gltf");
	//!m_Model = std::make_unique<Model>(m_Device.get(), "Assets/glTF/DamagedHelmet/DamagedHelmet.gltf");

	//m_Model = std::make_unique<Model>(m_Device.get(), "Assets/glTF/mathilda/scene.gltf");
	//m_Model = std::make_unique<Model>(m_Device.get(), "Assets/glTF/resto_ni_teo/scene.gltf");
	//m_Model = std::make_unique<Model>(m_Device.get(), "Assets/glTF/ice_cream_man/scene.gltf");
	//m_Model = std::make_unique<Model>(m_Device.get(), "Assets/glTF/lizard_mage/scene.gltf");
	//m_Model = std::make_unique<Model>(m_Device.get(), "Assets/glTF/pizza_ballerina/scene.gltf");
	//m_Model = std::make_unique<Model>(m_Device.get(), "Assets/glTF/realistic_armor/scene.gltf");
	//m_Model = std::make_unique<Model>(m_Device.get(), "Assets/glTF/3d_anime_character_girl_stylized/scene.gltf");
	//m_Model = std::make_unique<Model>(m_Device.get(), "Assets/glTF/smart_littel_robot/scene.gltf");
	//m_Model = std::make_unique<Model>(m_Device.get(), "Assets/glTF/stylized_planet/scene.gltf");
	//m_Model = std::make_unique<Model>(m_Device.get(), "Assets/glTF/fallout_ranger/scene.gltf");
	//m_Model = std::make_unique<Model>(m_Device.get(), "Assets/glTF/globophobia/scene.gltf");
	// With anims
	//m_Model = std::make_unique<Model>(m_Device.get(), "Assets/glTF/sgd162_idle_walk_cycle/scene.gltf");

	// TEST
	m_Model->SetSkyTexture(m_Skybox);

}

void Renderer::PreRender()
{
	// Dispatching Compute Shaders here
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

	m_Device->MoveToNextFrame();
}

void Renderer::RecordCommandLists(uint32_t CurrentFrame, Camera* pCamera)
{
	ThrowIfFailed(m_Device->m_CommandAllocators[m_Device->m_FrameIndex]->Reset());

	ThrowIfFailed(m_Device->GetCommandList()->Reset(m_Device->m_CommandAllocators[m_Device->m_FrameIndex].Get(), 
													m_ModelPipelineState.Get()));

	m_GUI->Begin();

	//m_Device->GetCommandList()->SetPipelineState(m_ModelPipelineState.Get());
	m_Device->GetCommandList()->SetPipelineState(SetPSO(m_SelectedPSO));
	m_Device->GetCommandList()->SetGraphicsRootSignature(m_ModelRootSignature.Get());
	m_Device->GetCommandList()->RSSetViewports(1, &m_Device->m_Viewport);
	m_Device->GetCommandList()->RSSetScissorRects(1, &m_Device->m_ViewportRect);

	TransitToRender();

	ID3D12DescriptorHeap* ppHeaps[] = { m_Device->m_cbvDescriptorHeap.GetHeap() };
	m_Device->GetCommandList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// RTV and Depth
	SetRenderTarget();
	
	// 3D
	{
		m_Model->Draw(pCamera);
	}
	SwitchPSO();

	m_Device->GetCommandList()->SetPipelineState(m_SkyboxPipelineState.Get());
	m_Device->GetCommandList()->SetGraphicsRootSignature(m_SkyboxRootSignature.Get());
	m_Skybox->Draw(pCamera);
	
	m_GUI->End(m_Device->GetCommandList());

	TransitToPresent();

	ThrowIfFailed(m_Device->GetCommandList()->Close());
}

void Renderer::OnResize()
{
	m_Device->WaitForGPU();
	m_Device->FlushGPU();
	ResizeBackbuffers();

	m_Device->WaitForGPU();
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

	m_Device->WaitForGPU();
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

ID3D12PipelineState* Renderer::SetPSO(int32_t Selected)
{
	switch (Selected)
	{
	case 0: 
		return m_ModelPipelineState.Get();
	case 1:
		return m_PBRPipelineState.Get();
	case 2:
		return m_PBRPipelineState2.Get();
	case 3:
		return m_PBRPipelineState3.Get();
	default:
		return m_ModelPipelineState.Get();
	}
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

void Renderer::SwitchPSO()
{
	ImGui::Begin("PSO");

	const std::array<const char*, 4> PSOs{
		"Base",
		"PBR",
		"PBR + Reflect",
		"PBR Test"
	};

	ImGui::ListBox("PSO", &Renderer::m_SelectedPSO, PSOs.data(), static_cast<int32_t>(PSOs.size()));

	ImGui::End();
}

void Renderer::InitModelPipeline()
{
	/*
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{ D3D_ROOT_SIGNATURE_VERSION_1_1 };
	if (FAILED(m_Device->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		// Rollback to 1_0
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}
	*/

	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges(9);
	// Vertex Constant Buffer
	ranges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	ranges.at(1).Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	// Pixel Constant Buffer
	ranges.at(2).Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	ranges.at(3).Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	// Textures
	ranges.at(4).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	ranges.at(5).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	ranges.at(6).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	ranges.at(7).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	// Skybox texture
	ranges.at(8).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	
	std::vector<CD3DX12_ROOT_PARAMETER1> params(9);
	// Vertex Constant Buffer
	params.at(0).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	params.at(1).InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	// Pixel Constant Buffer
	params.at(2).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	params.at(3).InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	// Textures
	params.at(4).InitAsDescriptorTable(1, &ranges.at(4), D3D12_SHADER_VISIBILITY_PIXEL);
	params.at(5).InitAsDescriptorTable(1, &ranges.at(5), D3D12_SHADER_VISIBILITY_PIXEL);
	params.at(6).InitAsDescriptorTable(1, &ranges.at(6), D3D12_SHADER_VISIBILITY_PIXEL);
	params.at(7).InitAsDescriptorTable(1, &ranges.at(7), D3D12_SHADER_VISIBILITY_PIXEL);
	// Skybox texture
	params.at(8).InitAsDescriptorTable(1, &ranges.at(8), D3D12_SHADER_VISIBILITY_PIXEL);

	auto layout			{ PSOUtils::CreateInputLayout() };
	auto rootFlags		{ PSOUtils::SetRootFlags() };

	PSOBuilder* builder{ new PSOBuilder() };
	builder->AddRanges(ranges);
	builder->AddParameters(params);
	builder->AddRootFlags(rootFlags);
	builder->AddInputLayout(layout);
	builder->AddSampler(0);
	builder->AddShaders("Assets/Shaders/Global_Vertex.hlsl", "Assets/Shaders/Normal_Mapping_Pixel.hlsl");
	builder->CreateRootSignature(m_Device->GetDevice(), m_ModelRootSignature.GetAddressOf(), L"Model Root Signature");

	builder->Create(m_Device->GetDevice(), m_ModelRootSignature.Get(), m_ModelPipelineState.GetAddressOf(), L"Model Pipeline State");
	
	builder->AddShaders("Assets/Shaders/Global_Vertex.hlsl", "Assets/Shaders/PBR_Pixel.hlsl");
	builder->Create(m_Device->GetDevice(), m_ModelRootSignature.Get(), m_PBRPipelineState.GetAddressOf(), L"Model Pipeline State");

	builder->AddShaders("Assets/Shaders/Global_Vertex.hlsl", "Assets/Shaders/PBR_Pixel_2.hlsl");
	builder->Create(m_Device->GetDevice(), m_ModelRootSignature.Get(), m_PBRPipelineState2.GetAddressOf(), L"Model Pipeline State");

	builder->AddShaders("Assets/Shaders/Global_Vertex.hlsl", "Assets/Shaders/PBR_Pixel_3.hlsl");
	builder->Create(m_Device->GetDevice(), m_ModelRootSignature.Get(), m_PBRPipelineState3.GetAddressOf(), L"Model Pipeline State");

	builder->Reset();

	// Skybox pipeline
	auto skyboxLayout{ PSOUtils::CreateSkyboxInputLayout() };
	std::vector<CD3DX12_DESCRIPTOR_RANGE1> skyboxRanges(2);
	skyboxRanges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	skyboxRanges.at(1).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	std::vector<CD3DX12_ROOT_PARAMETER1> skyboxParams(2);
	skyboxParams.at(0).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	skyboxParams.at(1).InitAsDescriptorTable(1, &skyboxRanges.at(1), D3D12_SHADER_VISIBILITY_PIXEL);

	builder->AddRanges(skyboxRanges);
	builder->AddParameters(skyboxParams);
	builder->AddSampler(0);
	builder->AddInputLayout(skyboxLayout);
	builder->AddShaders("Assets/Shaders/Sky/Skybox_VS.hlsl", "Assets/Shaders/Sky/Skybox_PS.hlsl");
	builder->CreateRootSignature(m_Device->GetDevice(), m_SkyboxRootSignature.GetAddressOf(), L"Skybox Root Signature");
	builder->Create(m_Device->GetDevice(), m_SkyboxRootSignature.Get(), m_SkyboxPipelineState.GetAddressOf(), L"Skybox Pipeline State");

}

void Renderer::OnDestroy()
{
	m_Device->WaitForGPU();

	::CloseHandle(m_Device->m_FenceEvent);
}
