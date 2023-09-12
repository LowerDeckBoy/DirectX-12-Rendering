#include "Renderer.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Core/Window.hpp"
#include "../Rendering/Camera.hpp"
#include "GraphicsPipelineState.hpp"
#include "../Utilities/Logger.hpp"
#include <array>

bool Renderer::bVsync	  = true;
bool Renderer::bDrawSky   = true;
bool Renderer::bDeferred  = true;
bool Renderer::bRaytrace  = false;

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

	m_ShaderManager = std::make_shared<ShaderManager>();
	
	InitPipelines();

	m_DeferredCtx = std::make_unique<DeferredContext>(m_DeviceCtx.get(), m_ShaderManager.get(), m_ModelRootSignature.Get());

	PreRender();
	LoadAssets();

	//m_RaytracingContext = std::make_unique<RaytracingContext>(m_DeviceCtx.get(), m_ShaderManager.get(), m_Camera, m_Models);

	m_DeviceCtx->ExecuteCommandList();

}

void Renderer::LoadAssets()
{
	// Lights
	m_PointLights = std::make_unique<PointLights>();
	m_PointLights->Create(m_DeviceCtx.get());
		
	// CBVs
	m_cbCamera = std::make_unique<ConstantBuffer<SceneConstData>>(m_DeviceCtx.get(), &m_cbSceneData);

	//m_Skybox = std::make_unique<Skybox>(m_DeviceCtx.get());
	
	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/sponza/Sponza.gltf", "Sponza"));
	m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/damaged_helmet/scene.gltf", "Helmet"));
	m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/ground/scene.gltf", "Ground"));
	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/suzanne/Suzanne.gltf", "Suzanne"));
	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/MetalRoughSpheres/MetalRoughSpheres.gltf", "Ligma"));

	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/ice_cream_man/scene.gltf", "ice_cream_man"));
	//m_Model = std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/sponza/Sponza.gltf");
	//m_Model = std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/damaged_helmet/scene.gltf", "Helmet");

	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/mathilda/scene.gltf"));
	//m_Model2 = std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/damaged_helmet/scene.gltf", "Sponza");

	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/resto_ni_teo/scene.gltf", "resto_ni_teo"));
	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/ice_cream_man/scene.gltf", "model"));
	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/witch_apprentice/scene.gltf"));
	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/lizard_mage/scene.gltf"));
	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/realistic_armor/scene.gltf"));
	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/smart_littel_robot/scene.gltf"));
	//m_Models.emplace_back(std::make_unique<Model>(m_DeviceCtx.get(), "Assets/glTF/globophobia/scene.gltf", "globophobia"));

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
	//m_IBL->Create(m_DeviceCtx.get(), "Assets/Textures/HDR/kloppenheim_06_puresky_4k.hdr");
	//m_IBL->Create(m_DeviceCtx.get(), "Assets/Textures/HDR/environment.hdr");

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

void Renderer::Render()
{
	RecordCommandLists(m_DeviceCtx->FRAME_INDEX);

	std::array<ID3D12CommandList*, 1> commandLists{ m_DeviceCtx->GetCommandList() };
	m_DeviceCtx->GetCommandQueue()->ExecuteCommandLists(static_cast<uint32_t>(commandLists.size()), commandLists.data());

	const HRESULT hResult{ m_DeviceCtx->GetSwapChain()->Present(bVsync ? 1 : 0, 0) };
	if (hResult == DXGI_ERROR_DEVICE_REMOVED || hResult == DXGI_ERROR_DEVICE_RESET)
	{
		::MessageBox(Window::GetHWND(), L"Device removed or Device reset", L"DXGI Error", MB_OK);
		throw std::logic_error("");
	}

	m_DeviceCtx->MoveToNextFrame();
}

void Renderer::DrawSkybox()
{
	if (!bDrawSky)
		return;

	SetPipelineState(m_SkyboxPipelineState.Get());
	SetRootSignature(m_SkyboxRootSignature.Get());

	m_IBL->Draw(m_Camera, m_DeviceCtx->FRAME_INDEX);

	//m_Skybox->Draw(m_Camera);
}

void Renderer::Forward()
{	
	SetRenderTarget();
	ClearRenderTarget();
	ClearDepthStencil();

	SetPipelineState(m_IBLPipelineState.Get());
	//SetRootSignature(m_ModelRootSignature.Get());
	//m_DeviceCtx->GetCommandList()->SetPipelineState(SetPSO(m_SelectedPSO));
	//SwitchPSO();

	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(6, m_DeviceCtx->GetDepthDescriptor().GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(7, m_IBL->m_OutputDescriptor.GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(8, m_IBL->m_IrradianceDescriptor.GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(9, m_IBL->m_SpecularDescriptor.GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(10, m_IBL->m_SpBRDFDescriptor.GetGPU());

	for (const auto& model : m_Models)
		model->Draw(m_Camera);

}

void Renderer::Deferred()
{
	const CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle(m_DeviceCtx->GetDepthHeap()->GetCPUDescriptorHandleForHeapStart());
	m_DeviceCtx->GetCommandList()->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, D3D12_MAX_DEPTH, 0, 0, nullptr);
	
	m_DeferredCtx->PassGBuffer(m_Camera, m_cbCamera.get(), m_Models);

	m_DeferredCtx->DrawToShadowMap(m_Camera, m_cbCamera.get(), m_PointLights.get(), m_Models, m_ModelRootSignature.Get());

	SetViewport();
	SetRenderTarget();
	ClearRenderTarget();

	m_DeferredCtx->PassShadows(m_Camera, m_cbCamera.get(), m_PointLights.get(), m_IBL.get(), m_Models);
	//m_DeferredCtx->PassLight(m_Camera, m_cbCamera.get(), m_IBL.get(), m_PointLights.get());

	m_DeferredCtx->DrawDeferredTargets();

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
	m_DeferredCtx->OnResize();
	
	if (m_RaytracingContext.get())
		m_RaytracingContext->OnResize();

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

void Renderer::SetViewport()
{
	m_DeviceCtx->GetCommandList()->RSSetViewports(1, &m_DeviceCtx->GetViewport());
	m_DeviceCtx->GetCommandList()->RSSetScissorRects(1, &m_DeviceCtx->GetScissor());
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

	SetViewport();

	m_GUI->Begin();

	m_DeviceCtx->GetCommandList()->SetGraphicsRootSignature(m_ModelRootSignature.Get());

	TransitToRender();
}

void Renderer::EndFrame()
{
	m_GUI->End(m_DeviceCtx->GetCommandList());

	TransitToPresent();

	ThrowIfFailed(m_DeviceCtx->GetCommandList()->Close(), "Failed to close Command List!");
}

ID3D12PipelineState* Renderer::SetPSO(int32_t Selected) noexcept
{
	switch (Selected)
	{
	case 0: 
		return m_IBLPipelineState.Get();
	case 1:
		return m_PBRPipelineState.Get();
	default:
		return m_IBLPipelineState.Get();
	}
}

void Renderer::SwitchPSO()
{
	ImGui::Begin("PSO");

	const std::array<const char*, 3> PSOs{
		"Image Based Lighting",
		"PBR",
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

	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges(6);
	{
		ranges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1024, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		ranges.at(1).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		ranges.at(2).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		ranges.at(3).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		ranges.at(4).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		ranges.at(5).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	}

	std::vector<CD3DX12_ROOT_PARAMETER1> params(11);
	{
		// Per Object matrices
		params.at(0).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
		// Camera Position
		params.at(1).InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
		// Pixel Constant Buffer
		params.at(2).InitAsConstantBufferView(0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
		params.at(3).InitAsConstantBufferView(1, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
		// Bindless
		params.at(4).InitAsDescriptorTable(1, &ranges.at(0), D3D12_SHADER_VISIBILITY_PIXEL);
		// Material indices
		params.at(5).InitAsConstants(4 * sizeof(int32_t), 0, 2);
		// Depth Texture
		params.at(6).InitAsDescriptorTable(1, &ranges.at(1));
		// Sky Texture references
		params.at(7).InitAsDescriptorTable(1, &ranges.at(2),  D3D12_SHADER_VISIBILITY_PIXEL);
		params.at(8).InitAsDescriptorTable(1, &ranges.at(3),  D3D12_SHADER_VISIBILITY_PIXEL);
		params.at(9).InitAsDescriptorTable(1, &ranges.at(4),  D3D12_SHADER_VISIBILITY_PIXEL);
		params.at(10).InitAsDescriptorTable(1, &ranges.at(5), D3D12_SHADER_VISIBILITY_PIXEL);
	}

	auto layout{ PSOUtils::CreateInputLayout() };
	const auto rootFlags{ PSOUtils::SetRootFlags() };

	PSOBuilder* builder{ new PSOBuilder() };
	// PBR + IBL PSO
	{
		builder->AddShaderManger(m_ShaderManager.get());
		builder->AddRanges(ranges);
		builder->AddParameters(params);
		builder->AddRootFlags(rootFlags);
		builder->AddInputLayout(layout);
		builder->AddSampler(0);
		builder->AddSampler(1, 0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS);
		builder->SetCullMode(D3D12_CULL_MODE_BACK);
		
		builder->CreateRootSignature(m_DeviceCtx->GetDevice(), m_ModelRootSignature.ReleaseAndGetAddressOf(), L"Model Root Signature");

		builder->AddShaders("Assets/Shaders/Forward/Global_Vertex.hlsl", "Assets/Shaders/Forward/PBR_Pixel.hlsl");
		builder->Create(m_DeviceCtx->GetDevice(), m_ModelRootSignature.Get(), m_PBRPipelineState.GetAddressOf(), L"Model Pipeline State");

		builder->AddShaders("Assets/Shaders/Forward/Global_Vertex.hlsl", "Assets/Shaders/Forward/IBL_Pixel.hlsl");
		builder->Create(m_DeviceCtx->GetDevice(), m_ModelRootSignature.Get(), m_IBLPipelineState.GetAddressOf(), L"Model Pipeline State IBL");
	}
	
	builder->Reset();

	// Skybox pipeline
	{
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

		builder->CreateRootSignature(m_DeviceCtx->GetDevice(), m_SkyboxRootSignature.GetAddressOf(), L"Skybox Root Signature");
		builder->Create(m_DeviceCtx->GetDevice(), m_SkyboxRootSignature.Get(), m_SkyboxPipelineState.GetAddressOf(), L"Skybox Pipeline State");

		builder->Reset();
	}

	builder->Reset();
	builder = nullptr;
	delete builder;
}

void Renderer::Release()
{
	m_DeviceCtx->WaitForGPU();
	m_DeviceCtx->FlushGPU();

	// PSO
	SAFE_RELEASE(m_SkyboxRootSignature);
	SAFE_RELEASE(m_SkyboxPipelineState);

	SAFE_RELEASE(m_ModelRootSignature);
	SAFE_RELEASE(m_PBRPipelineState);
	SAFE_RELEASE(m_IBLPipelineState);

	::CloseHandle(m_DeviceCtx->GetFenceEvent());

	Logger::Log("Renderer released.");
}

void Renderer::DrawGUI()
{
	m_PointLights->DrawGUI();

	for (const auto& model : m_Models)
		model->DrawGUI();

}
