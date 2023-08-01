#include "Renderer.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Core/Window.hpp"
#include "../Rendering/Camera.hpp"
#include "GraphicsPipelineState.hpp"
#include "../Utilities/Logger.hpp"
#include <array>

bool Renderer::bDrawSky   = true;
bool Renderer::bDeferred  = true;

Renderer::~Renderer()
{
	Release();
}

void Renderer::Initialize(Camera* pCamera)
{
	m_DeviceCtx = std::make_unique<DeviceContext>();
	m_DeviceCtx->Initialize();
	m_GUI = std::make_unique<Editor>();
	m_GUI->Initialize(m_DeviceCtx.get(), pCamera);

	assert(m_Camera = pCamera);

	m_ShaderManager = std::make_unique<ShaderManager>();

	InitPipelines();

	m_ScreenQuad = std::make_unique<ScreenQuad>(m_DeviceCtx.get());

	LoadAssets();
	PreRender();

	m_DeviceCtx->ExecuteCommandList();

}

void Renderer::LoadAssets()
{
	// Lights
	m_PointLights = std::make_unique<PointLight>();
	m_PointLights->Create(m_DeviceCtx.get());
		
	// CBVs
	m_cbCamera = std::make_unique<ConstantBuffer<SceneConstData>>(m_DeviceCtx.get(), &m_cbSceneData);

	//m_Skybox = std::make_unique<Skybox>(m_DeviceCtx.get());
	
	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/sponza/Sponza.gltf", "Sponza"));
	m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/damaged_helmet/scene.gltf", "Helmet"));

	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/ice_cream_man/scene.gltf", "ice_cream_man"));
	//m_Model = std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/sponza/Sponza.gltf");
	//m_Model = std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/damaged_helmet/scene.gltf", "Helmet");

	//m_Model = std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/mathilda/scene.gltf");
	//m_Model2 = std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/damaged_helmet/scene.gltf", "Sponza");

	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/resto_ni_teo/scene.gltf", "resto_ni_teo"));
	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/ice_cream_man/scene.gltf", "model"));
	//m_Model = std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/witch_apprentice/scene.gltf");
	//m_Model = std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/lizard_mage/scene.gltf");
	//m_Model = std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/realistic_armor/scene.gltf");
	//m_Model = std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/smart_littel_robot/scene.gltf");
	//m_Model = std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/globophobia/scene.gltf");

	// With anims
	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/sgd162_idle_walk_cycle/scene.gltf"));
	//m_Model = std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/sgd162_idle_walk_run_cycle/scene.gltf");
	//m_Model = std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/chaman_ti-pche_3_animations/scene.gltf");

}

void Renderer::PreRender()
{
	// Dispatching Compute Shaders here

	m_IBL = std::make_unique<ImageBasedLighting>();
	m_IBL->Create(m_DeviceCtx.get(), "Assets/Textures/HDR/newport_loft.hdr");
	//m_IBL->Create(m_DeviceCtx.get(), "Assets/Textures/HDR/environment.hdr");
	//m_IBL->Create(m_DeviceCtx.get(), "Assets/Textures/HDR/kiara_1_dawn_4k.hdr");
	//m_IBL->Create(m_DeviceCtx.get(), "Assets/Textures/HDR/nagoya_wall_path_4k.hdr");
	//m_IBL->Create(m_DeviceCtx.get(), "Assets/Textures/HDR/photo_studio_london_hall_4k.hdr");
	//m_IBL->Create(m_DeviceCtx.get(), "Assets/Textures/HDR/spaichingen_hill_4k.hdr");
	//m_IBL->Create(m_DeviceCtx.get(), "Assets/Textures/HDR/studio_small_08_4k.hdr");

}

void Renderer::Update()
{
	m_PointLights->UpdateLights();

	// Update CBVs
	m_cbCamera->Update({ m_Camera->GetPosition(),
						 m_Camera->GetView(),
						 m_Camera->GetProjection(),
						 XMMatrixTranspose(XMMatrixInverse(nullptr, m_Camera->GetView())),
						 XMMatrixTranspose(XMMatrixInverse(nullptr, m_Camera->GetProjection())),
						 XMFLOAT2(m_DeviceCtx->GetViewport().Width, m_DeviceCtx->GetViewport().Height) },
						 m_DeviceCtx->FRAME_INDEX);

}

void Renderer::Draw()
{
	RecordCommandLists(m_DeviceCtx->FRAME_INDEX);

	std::array<ID3D12CommandList*, 1> commandLists{ m_DeviceCtx->GetCommandList() };
	m_DeviceCtx->GetCommandQueue()->ExecuteCommandLists(static_cast<uint32_t>(commandLists.size()), commandLists.data());

	const HRESULT hResult{ m_DeviceCtx->GetSwapChain()->Present(1, 0) };
	if (hResult == DXGI_ERROR_DEVICE_REMOVED || hResult == DXGI_ERROR_DEVICE_RESET)
	{
		::MessageBox(Window::GetHWND(), L"Device removed or Device reset", L"DXGI Error", MB_OK);
		throw std::logic_error("");
	}

	m_DeviceCtx->MoveToNextFrame();
}

void Renderer::DrawSkybox()
{
	if (bDrawSky)
	{
		SetPipelineState(m_SkyboxPipelineState.Get());
		SetRootSignature(m_SkyboxRootSignature.Get());

		m_IBL->Draw(m_Camera, m_DeviceCtx->FRAME_INDEX);

		//m_Skybox->Draw(m_Camera);
	}
}

void Renderer::Forward()
{	
	SetRenderTarget();
	ClearRenderTarget();
	ClearDepthStencil();

	m_DeviceCtx->GetCommandList()->SetPipelineState(SetPSO(m_SelectedPSO));
	SwitchPSO();

	SetRootSignature(m_ModelRootSignature.Get());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(6, m_IBL->m_OutputDescriptor.GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(7, m_IBL->m_PrefilteredDescriptor.GetGPU());

	for (const auto& model : m_Models)
		model->Draw(m_Camera);

	//m_Model->Draw(m_Camera);
	//m_Model2->Draw(m_Camera);

}

void Renderer::Deferred()
{
	const CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle(m_DeviceCtx->GetDepthHeap()->GetCPUDescriptorHandleForHeapStart());
	m_DeviceCtx->GetCommandList()->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	
	PassGBuffer(m_Camera);

	ImGui::Begin("Render Targets");
	const auto rtv0 = m_RTSRVDescs.at(0).GetGPU();
	const auto rtv1 = m_RTSRVDescs.at(1).GetGPU();
	const auto rtv2 = m_RTSRVDescs.at(2).GetGPU();
	const auto rtv3 = m_RTSRVDescs.at(3).GetGPU();
	const auto rtv4 = m_RTSRVDescs.at(4).GetGPU();
	const auto depth = m_DeviceCtx->GetDepthDescriptor().GetGPU();

	ImGui::Image(reinterpret_cast<ImTextureID>(rtv0.ptr), { 500, 350 });
	ImGui::SameLine();
	ImGui::Image(reinterpret_cast<ImTextureID>(rtv1.ptr), { 500, 350 });
	ImGui::SameLine();
	ImGui::Image(reinterpret_cast<ImTextureID>(rtv2.ptr), { 500, 350 });
	ImGui::Image(reinterpret_cast<ImTextureID>(rtv3.ptr), { 500, 350 });
	ImGui::SameLine();
	ImGui::Image(reinterpret_cast<ImTextureID>(rtv4.ptr), { 500, 350 });
	ImGui::SameLine();
	ImGui::Image(reinterpret_cast<ImTextureID>(depth.ptr), { 500, 350 });
	
	ImGui::End();

	SetRenderTarget();
	ClearRenderTarget();

	PassLight(m_Camera);

}

void Renderer::RecordCommandLists(uint32_t CurrentFrame)
{
	BeginFrame();
	
	SetHeap(m_DeviceCtx->GetMainHeap()->GetHeapAddressOf());

	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(4, m_DeviceCtx->GetMainHeap()->GetHeap()->GetGPUDescriptorHandleForHeapStart());
	//m_DeviceCtx->GetCommandList()->SetGraphicsRootConstantBufferView(1, m_cbCamera->GetBuffer(m_DeviceCtx->FRAME_INDEX)->GetGPUVirtualAddress());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootConstantBufferView(3, m_PointLights->m_cbPointLights->GetBuffer(m_DeviceCtx->FRAME_INDEX)->GetGPUVirtualAddress());

	if (bDeferred)
	{
		Deferred();
	}
	else
	{
		Forward();
	}

	DrawSkybox();
	
	DrawGUI();
	
	EndFrame();
}

void Renderer::OnResize()
{
	m_DeviceCtx->OnResize();
	CreateDeferredRTVs();

	m_DeviceCtx->WaitForGPU();
	m_DeviceCtx->FlushGPU();
	m_DeviceCtx->ExecuteCommandList();
}

void Renderer::SetHeap(ID3D12DescriptorHeap** ppHeap)
{
	m_DeviceCtx->GetCommandList()->SetDescriptorHeaps(1, ppHeap);
}

void Renderer::SetRootSignature(ID3D12RootSignature* pRootSignature)
{
	m_DeviceCtx->GetCommandList()->SetGraphicsRootSignature(pRootSignature);
}

void Renderer::SetPipelineState(ID3D12PipelineState* pPipelineState)
{
	m_DeviceCtx->GetCommandList()->SetPipelineState(pPipelineState);
}

void Renderer::SetRenderTarget()
{
	const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_DeviceCtx->GetRenderTargetHeap()->GetCPUDescriptorHandleForHeapStart(), m_DeviceCtx->FRAME_INDEX, m_DeviceCtx->GetDescriptorSize());
	const CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle(m_DeviceCtx->GetDepthHeap()->GetCPUDescriptorHandleForHeapStart());

	//TransitToRender();
	m_DeviceCtx->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, TRUE, &depthHandle);
}

void Renderer::ClearRenderTarget()
{
	const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_DeviceCtx->GetRenderTargetHeap()->GetCPUDescriptorHandleForHeapStart(), m_DeviceCtx->FRAME_INDEX, m_DeviceCtx->GetDescriptorSize());
	m_DeviceCtx->GetCommandList()->ClearRenderTargetView(rtvHandle, m_ClearColor.data(), 0, nullptr);
}

void Renderer::ClearDepthStencil()
{
	const CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle(m_DeviceCtx->GetDepthHeap()->GetCPUDescriptorHandleForHeapStart());
	m_DeviceCtx->GetCommandList()->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Renderer::TransitToRender()
{
	const auto presentToRender = CD3DX12_RESOURCE_BARRIER::Transition(m_DeviceCtx->GetRenderTarget(),
																	D3D12_RESOURCE_STATE_PRESENT,
																	D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &presentToRender);
}

void Renderer::TransitToPresent(D3D12_RESOURCE_STATES StateBefore)
{
	const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_DeviceCtx->GetRenderTarget(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &barrier);
}

void Renderer::BeginFrame()
{
	m_DeviceCtx->ResetCommandList();

	const auto viewport{ m_DeviceCtx->GetViewport() };
	const auto scissor{ m_DeviceCtx->GetScissor() };
	m_DeviceCtx->GetCommandList()->RSSetViewports(1, &viewport);
	m_DeviceCtx->GetCommandList()->RSSetScissorRects(1, &scissor);

	m_DeviceCtx->GetCommandList()->SetGraphicsRootSignature(m_ModelRootSignature.Get());

	m_GUI->Begin();

	TransitToRender();
}

void Renderer::EndFrame()
{
	m_GUI->End(m_DeviceCtx->GetCommandList());

	TransitToPresent();

	ThrowIfFailed(m_DeviceCtx->GetCommandList()->Close());
}

ID3D12PipelineState* Renderer::SetPSO(int32_t Selected) noexcept
{
	switch (Selected)
	{
	case 0: 
		return m_PBRPipelineState.Get();
		//return m_DeferredPSO.Get();
	case 1:
		return m_PBRPipelineState2.Get();
	default:
		return m_PBRPipelineState.Get();
	}
}

void Renderer::SwitchPSO()
{
	ImGui::Begin("PSO");

	const std::array<const char*, 2> PSOs{
		"PBR",
		"Image Based Lighting",
	};

	ImGui::ListBox("PSO", &Renderer::m_SelectedPSO, PSOs.data(), static_cast<int32_t>(PSOs.size()));

	ImGui::End();
}

void Renderer::InitPipelines()
{
	/*
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{ D3D_ROOT_SIGNATURE_VERSION_1_1 };
	if (FAILED(m_DeviceCtx->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		// Rollback to 1_0
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}
	*/

	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges(3);
	ranges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4096, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	ranges.at(1).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	ranges.at(2).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

	std::vector<CD3DX12_ROOT_PARAMETER1> params(8);
	// Per Object matrices
	params.at(0).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	// Camera Position
	params.at(1).InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
	// Pixel Constant Buffer
	params.at(2).InitAsConstantBufferView(0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	params.at(3).InitAsConstantBufferView(1, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	// Bindless
	params.at(4).InitAsDescriptorTable(1, &ranges.at(0), D3D12_SHADER_VISIBILITY_ALL);
	// Material indices
	params.at(5).InitAsConstants(4 * sizeof(int32_t), 0, 2);
	// Sky Texture references
	params.at(6).InitAsDescriptorTable(1, &ranges.at(1), D3D12_SHADER_VISIBILITY_ALL);
	params.at(7).InitAsDescriptorTable(1, &ranges.at(2), D3D12_SHADER_VISIBILITY_ALL);

	auto layout{ PSOUtils::CreateInputLayout() };
	const auto rootFlags{ PSOUtils::SetRootFlags() };

	PSOBuilder* builder{ new PSOBuilder() };
	builder->AddShaderManger(m_ShaderManager.get());
	builder->AddRanges(ranges);
	builder->AddParameters(params);
	builder->AddRootFlags(rootFlags);
	builder->AddInputLayout(layout);
	builder->AddSampler(0);
	
	builder->CreateRootSignature(m_DeviceCtx->GetDevice(), m_ModelRootSignature.GetAddressOf(), L"Model Root Signature");

	builder->AddShaders("Assets/Shaders/Forward/Global_Vertex.hlsl", "Assets/Shaders/Forward/PBR_Pixel.hlsl");
	builder->Create(m_DeviceCtx->GetDevice(), m_ModelRootSignature.Get(), m_PBRPipelineState.GetAddressOf(), L"Model Pipeline State");

	builder->AddShaders("Assets/Shaders/Forward/Global_Vertex.hlsl", "Assets/Shaders/Forward/PBR_Pixel_Test.hlsl");
	//builder->SetFillMode(D3D12_FILL_MODE_WIREFRAME);
	builder->Create(m_DeviceCtx->GetDevice(), m_ModelRootSignature.Get(), m_PBRPipelineState2.GetAddressOf(), L"Model Pipeline State");

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
	builder->SetCullMode(D3D12_CULL_MODE_FRONT);
	// IBL
	//builder->AddShaders("Assets/Shaders/Sky/Skybox_VS.hlsl", "Assets/Shaders/Sky/Skybox_Test.hlsl");
	builder->CreateRootSignature(m_DeviceCtx->GetDevice(), m_SkyboxRootSignature.GetAddressOf(), L"Skybox Root Signature");
	builder->Create(m_DeviceCtx->GetDevice(), m_SkyboxRootSignature.Get(), m_SkyboxPipelineState.GetAddressOf(), L"Skybox Pipeline State");

	builder->Reset();

	// Deferred Root Signature
	{
		std::vector<CD3DX12_DESCRIPTOR_RANGE1> deferredRanges(8);
		deferredRanges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(1).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(2).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(3).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(4).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(5).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(6).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(7).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		
		std::vector<CD3DX12_ROOT_PARAMETER1> deferredParams(10);
		// GBuffer output textures
		// Base Color
		deferredParams.at(0).InitAsDescriptorTable(1, &deferredRanges.at(0), D3D12_SHADER_VISIBILITY_PIXEL);
		// Normal
		deferredParams.at(1).InitAsDescriptorTable(1, &deferredRanges.at(1), D3D12_SHADER_VISIBILITY_PIXEL);
		// Metallic
		deferredParams.at(2).InitAsDescriptorTable(1, &deferredRanges.at(2), D3D12_SHADER_VISIBILITY_PIXEL);
		// Emissive
		deferredParams.at(3).InitAsDescriptorTable(1, &deferredRanges.at(3), D3D12_SHADER_VISIBILITY_PIXEL);
		// Positions
		deferredParams.at(4).InitAsDescriptorTable(1, &deferredRanges.at(4), D3D12_SHADER_VISIBILITY_PIXEL);
		// Depth
		deferredParams.at(5).InitAsDescriptorTable(1, &deferredRanges.at(5), D3D12_SHADER_VISIBILITY_PIXEL);
		// Skybox
		deferredParams.at(6).InitAsDescriptorTable(1, &deferredRanges.at(6), D3D12_SHADER_VISIBILITY_PIXEL);
		// Image Based Lighting
		deferredParams.at(7).InitAsDescriptorTable(1, &deferredRanges.at(7), D3D12_SHADER_VISIBILITY_PIXEL);
		// CBVs
		deferredParams.at(8).InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
		deferredParams.at(9).InitAsConstantBufferView(1, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

		builder->AddRanges(deferredRanges);
		builder->AddParameters(deferredParams);
		builder->AddSampler(0);
		builder->CreateRootSignature(m_DeviceCtx->GetDevice(), m_DeferredRootSignature.GetAddressOf(), L"Deferred Root Signature");

		builder->Reset();
	}

	// Deferred PSO
	{
		//
		IDxcBlob* const deferredVertex = m_ShaderManager->CreateDXIL("Assets/Shaders/Deferred/GBuffer_Vertex.hlsl", L"vs_6_0");
		IDxcBlob* const deferredPixel  = m_ShaderManager->CreateDXIL("Assets/Shaders/Deferred/GBuffer_Pixel.hlsl",  L"ps_6_0");
		//
		D3D12_GRAPHICS_PIPELINE_STATE_DESC deferredDesc{};
		deferredDesc.pRootSignature = m_ModelRootSignature.Get();
		deferredDesc.InputLayout = { layout.data(), static_cast<uint32_t>(layout.size()) };
		deferredDesc.VS = CD3DX12_SHADER_BYTECODE(deferredVertex->GetBufferPointer(), deferredVertex->GetBufferSize());
		deferredDesc.PS = CD3DX12_SHADER_BYTECODE(deferredPixel->GetBufferPointer(), deferredPixel->GetBufferSize());
		deferredDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		deferredDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		deferredDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		deferredDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		deferredDesc.SampleMask = UINT_MAX;
		deferredDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		deferredDesc.NumRenderTargets = m_DeferredRTVCount;
		deferredDesc.RTVFormats[0] = m_RTVFormats.at(0);
		deferredDesc.RTVFormats[1] = m_RTVFormats.at(1);
		deferredDesc.RTVFormats[2] = m_RTVFormats.at(2);
		deferredDesc.RTVFormats[3] = m_RTVFormats.at(3);
		deferredDesc.RTVFormats[4] = m_RTVFormats.at(4);
		deferredDesc.RTVFormats[5] = m_RTVFormats.at(5);
		deferredDesc.DSVFormat = m_DeviceCtx->GetDepthFormat();
		deferredDesc.SampleDesc.Count = 1;

		m_DeviceCtx->GetDevice()->CreateGraphicsPipelineState(&deferredDesc, IID_PPV_ARGS(m_DeferredPSO.GetAddressOf()));

		// Light Pass PSO
		auto screenQuadLayout{ PSOUtils::CreateScreenQuadLayout() };

		IDxcBlob* const quadVertex = m_ShaderManager->CreateDXIL("Assets/Shaders/Deferred/Deferred_Vertex.hlsl", L"vs_6_0");
		IDxcBlob* const quadPixel  = m_ShaderManager->CreateDXIL("Assets/Shaders/Deferred/Deferred_Pixel.hlsl", L"ps_6_0");

		D3D12_GRAPHICS_PIPELINE_STATE_DESC lightDesc{};
		lightDesc.pRootSignature = m_DeferredRootSignature.Get();
		lightDesc.VS = CD3DX12_SHADER_BYTECODE(quadVertex->GetBufferPointer(), quadVertex->GetBufferSize());
		lightDesc.PS = CD3DX12_SHADER_BYTECODE(quadPixel->GetBufferPointer(), quadPixel->GetBufferSize());
		lightDesc.InputLayout = { screenQuadLayout.data(), static_cast<uint32_t>(screenQuadLayout.size()) };
		lightDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		lightDesc.DepthStencilState.DepthEnable = false;
		lightDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		lightDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		lightDesc.RasterizerState.DepthClipEnable = false;
		lightDesc.SampleMask = UINT_MAX;
		lightDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		lightDesc.NumRenderTargets = 1;
		lightDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		lightDesc.DSVFormat = m_DeviceCtx->GetDepthFormat();
		lightDesc.SampleDesc = { 1, 0 };
		
		m_DeviceCtx->GetDevice()->CreateGraphicsPipelineState(&lightDesc, IID_PPV_ARGS(m_DeferredLightPSO.GetAddressOf()));
	}
	
	builder = nullptr;
	delete builder;
}

void Renderer::Release()
{
	// PSO
	SAFE_RELEASE(m_SkyboxRootSignature);
	SAFE_RELEASE(m_SkyboxPipelineState);

	SAFE_RELEASE(m_ModelRootSignature);
	SAFE_RELEASE(m_ModelPipelineState);
	SAFE_RELEASE(m_PBRPipelineState);
	SAFE_RELEASE(m_PBRPipelineState2);

	m_DeviceCtx->WaitForGPU();

	::CloseHandle(m_DeviceCtx->GetFenceEvent());

	Logger::Log("Renderer released.");
}

void Renderer::CreateDeferredRTVs()
{
	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.MipLevels = 1;
	desc.DepthOrArraySize = 1;
	desc.Width  = static_cast<uint64_t>(m_DeviceCtx->GetViewport().Width);
	desc.Height = static_cast<uint32_t>(m_DeviceCtx->GetViewport().Height);
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	desc.SampleDesc = { 1, 0 };

	D3D12_CLEAR_VALUE clear{};
	clear.Color[0] = m_ClearColor.at(0);
	clear.Color[1] = m_ClearColor.at(1);
	clear.Color[2] = m_ClearColor.at(2);
	clear.Color[3] = m_ClearColor.at(3);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = m_DeferredRTVCount;
	m_DeviceCtx->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DefRTVHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_DefRTVHeap->GetCPUDescriptorHandleForHeapStart());
	const auto heapProps{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
	for (uint32_t i = 0; i < m_DeferredRTVCount; i++)
	{
		desc.Format  = m_RTVFormats.at(i);
		clear.Format = m_RTVFormats.at(i);
		ThrowIfFailed(m_DeviceCtx->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, &clear, IID_PPV_ARGS(m_RTVTextures[i].ReleaseAndGetAddressOf())));

		std::wstring nm{ L"Deferred RTV" + std::to_wstring(i) };
		LPCWSTR name{ nm.c_str() };
		m_RTVTextures[i]->SetName(name);

		// RTVs
		m_RTVDescriptors.at(i) = rtvHandle;
		m_DeviceCtx->GetDevice()->CreateRenderTargetView(m_RTVTextures[i].Get(), &rtvDesc, m_RTVDescriptors.at(i));
		rtvHandle.Offset(1, 32);

		// SRVs
		m_DeviceCtx->GetMainHeap()->Allocate(m_RTSRVDescs.at(i));
		m_DeviceCtx->GetDevice()->CreateShaderResourceView(m_RTVTextures[i].Get(), &srvDesc, m_RTSRVDescs.at(i).GetCPU());
	}

}

void Renderer::PassGBuffer(Camera* pCamera)
{
	for (uint32_t i = 0; i < m_DeferredRTVCount; i++)
	{
		const auto toRender{ CD3DX12_RESOURCE_BARRIER::Transition(m_RTVTextures[i].Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET) };
		m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &toRender);
	}

	//clear
	for (uint32_t i = 0; i < m_DeferredRTVCount; i++)
		m_DeviceCtx->GetCommandList()->ClearRenderTargetView(m_RTVDescriptors.at(i), m_ClearColor.data(), 0, nullptr);

	SetPipelineState(m_DeferredPSO.Get());
	//SetRootSignature(m_DeferredRootSignature.Get());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootConstantBufferView(1, m_cbCamera->GetBuffer(m_DeviceCtx->FRAME_INDEX)->GetGPUVirtualAddress());

	const CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle(m_DeviceCtx->GetDepthHeap()->GetCPUDescriptorHandleForHeapStart());
	m_DeviceCtx->GetCommandList()->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_DeviceCtx->GetCommandList()->OMSetRenderTargets(m_DeferredRTVCount, m_RTVDescriptors.data(), false, &depthHandle);

	for (const auto& model : m_Models)
		model->Draw(pCamera);

	//m_Model->Draw(pCamera);
	//m_Model2->Draw(pCamera);

	for (uint32_t i = 0; i < m_DeferredRTVCount; i++)
	{
		const auto toRead{ CD3DX12_RESOURCE_BARRIER::Transition(m_RTVTextures[i].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ) };
		m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &toRead);
	}
	
}

void Renderer::PassLight(Camera* pCamera)
{
	const auto depthWriteToRead{ CD3DX12_RESOURCE_BARRIER::Transition(m_DeviceCtx->GetDepthStencil(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ) };
	m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &depthWriteToRead);

	// Set PSO and Root for deferred
	SetPipelineState(m_DeferredLightPSO.Get());
	SetRootSignature(m_DeferredRootSignature.Get());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(0, m_RTSRVDescs.at(0).GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(1, m_RTSRVDescs.at(1).GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(2, m_RTSRVDescs.at(2).GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(3, m_RTSRVDescs.at(3).GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(4, m_RTSRVDescs.at(4).GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(5, m_DeviceCtx->GetDepthDescriptor().GetGPU());
	//m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(6, m_Skybox->GetTex().m_Descriptor.GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(6, m_IBL->m_OutputDescriptor.GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(7, m_IBL->m_PrefilteredDescriptor.GetGPU());

	m_DeviceCtx->GetCommandList()->SetGraphicsRootConstantBufferView(8, m_cbCamera->GetBuffer(m_DeviceCtx->FRAME_INDEX)->GetGPUVirtualAddress());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootConstantBufferView(9, m_PointLights->m_cbPointLights->GetBuffer(m_DeviceCtx->FRAME_INDEX)->GetGPUVirtualAddress());

	m_ScreenQuad->Draw(m_DeviceCtx->GetCommandList());

	const auto depthReadToWrite{ CD3DX12_RESOURCE_BARRIER::Transition(m_DeviceCtx->GetDepthStencil(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE) };
	m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &depthReadToWrite);

}

void Renderer::DrawGUI()
{
	m_PointLights->DrawGUI();

}
