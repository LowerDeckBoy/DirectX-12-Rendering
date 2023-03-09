#include "Renderer.hpp"
#include "../Utils/Utils.hpp"
#include "../Core/Window.hpp"
#include "../Rendering/Camera.hpp"

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

	//D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	//heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	
	CreateDepthStencil();

	// change to creating pipeline state and compiling shaders
	//InitTriangle();
	InitModelPipeline();


	//m_Device->CreateCommandList(m_PipelineState.Get());
	m_Device->CreateCommandList(m_ModelPipelineState.Get());
	m_Device->GetCommandList()->SetPipelineState(m_ModelPipelineState.Get());
	//LoadAssets("Assets/Textures/shrek.jpg");

	//m_Cube.Initialize(m_Device.get());

	//m_Model.Initialize(m_Device.get(), "Assets/glTF/cube/Cube.gltf");
	//m_Model.Initialize(m_Device.get(), "Assets/glTF/psyduck/scene.gltf");
	//m_Model.Initialize(m_Device.get(), "Assets/glTF/mathilda/scene.gltf");
	m_Model.Initialize(m_Device.get(), "Assets/glTF/fallout_ranger/scene.gltf");


	ThrowIfFailed(m_Device->GetCommandList()->Close());
	ID3D12CommandList* ppCommandLists[] = { m_Device->GetCommandList() };
	m_Device->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	WaitForGPU();

	//m_Cube.Initialize(m_Device.get());

}

void Renderer::InitPipelineState()
{
}

void Renderer::Update(XMMATRIX ViewProj)
{

	//const XMMATRIX world{ XMMatrixIdentity() };
	////const XMMATRIX world{ m_Model. };
	//m_cbPerObject.WVP = world * ViewProj;
	//std::memcpy(m_ConstBuffer.pDataBegin, &m_cbPerObject, sizeof(m_cbPerObject));

	//m_Model.m_cbData.WVP = m_Model.

}

void Renderer::Draw(Camera* pCamera)
{
	RecordCommandLists(pCamera);

	ID3D12CommandList* commandLists[]{ m_Device->GetCommandList() };
	m_Device->GetCommandQueue()->ExecuteCommandLists(_countof(commandLists), commandLists);

	HRESULT hResult{ m_Device->GetSwapChain()->Present(0, 0) };
	if (hResult == DXGI_ERROR_DEVICE_REMOVED || hResult == DXGI_ERROR_DEVICE_RESET)
	{
		::MessageBox(Window::GetHWND(), L"Device removed or Device reset", L"DXGI Error", MB_OK);
		throw std::exception();
	}

	MoveToNextFrame();
}

void Renderer::RecordCommandLists(Camera* pCamera)
{
	ThrowIfFailed(m_Device->m_CommandAllocators[m_Device->m_FrameIndex]->Reset());
	ThrowIfFailed(m_Device->GetCommandList()->Reset(
		m_Device->m_CommandAllocators[m_Device->m_FrameIndex].Get(), 
		m_ModelPipelineState.Get()));

	m_GUI->Begin();

	m_Device->GetCommandList()->RSSetViewports(1, &m_Device->m_Viewport);
	m_Device->GetCommandList()->RSSetScissorRects(1, &m_Device->m_ViewportRect);

	ID3D12DescriptorHeap* ppHeaps[] = { m_Device->m_cbvDescriptorHeap.m_Heap.Get(), m_Device->m_SamplerHeap.Get() };
	//ID3D12DescriptorHeap* ppHeaps[] = { m_Device->m_cbvDescriptorHeaps.at(m_Device->m_FrameIndex).m_Heap.Get(), m_Device->m_SamplerHeap.Get()};
	m_Device->GetCommandList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	TransitToRender();

	// RTV and Depth
	SetRenderTarget();
	
	m_Device->GetCommandList()->SetGraphicsRootSignature(m_ModelRootSignature.Get());
	//m_Cube.Draw();
	//m_Model.Draw(pCamera);
	m_Model.DrawMeshes(pCamera);

	m_Model.DrawGUI();
	m_GUI->End(m_Device->GetCommandList());

	TransitToPresent();

	ThrowIfFailed(m_Device->GetCommandList()->Close());

}

void Renderer::OnResize()
{
	WaitForGPU();
	FlushGPU();
	ResizeBackbuffers();
	// TODO: Investigate
	// For some reason Idle needs to be done twice
	// 
	WaitForGPU();
}

void Renderer::ResizeBackbuffers()
{
	if (!m_Device->GetDevice() || !m_Device->GetSwapChain() || !m_Device->m_CommandAllocators[m_Device->m_FrameIndex])
		throw std::exception();

	m_Device->m_CommandAllocators[m_Device->m_FrameIndex]->Reset();
	//m_Device->GetCommandList()->Reset(m_Device->m_CommandAllocators[m_Device->m_FrameIndex].Get(), nullptr);
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

void Renderer::OnDestroy()
{
	WaitForGPU();
	//WaitForPreviousFrame();

	//SafeRelease(m_PipelineState);
	//SafeRelease(m_VertexBuffer);
}

void Renderer::SetRenderTarget()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_Device->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(), m_Device->m_FrameIndex, m_Device->GetDescriptorSize());
	CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle(m_DepthHeap.Get()->GetCPUDescriptorHandleForHeapStart());
	m_Device->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &depthHandle);
	//m_Device->GetCommandList()->depth

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

	D3D12_RESOURCE_DESC depthDesc{};
	depthDesc.Width = static_cast<uint64_t>(m_Device->GetViewport().Width);
	depthDesc.Height = static_cast<uint32_t>(m_Device->GetViewport().Height);
	depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthDesc.MipLevels = 1;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.DepthOrArraySize = 1;
	depthDesc.Alignment = 0;
	depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	//depthDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	

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

	auto cpuHandle{ m_DepthHeap.Get()->GetCPUDescriptorHandleForHeapStart() };
	m_Device->GetDevice()->CreateDepthStencilView(m_DepthStencil.Get(), &dsView, cpuHandle);
	m_DepthStencil.Get()->SetName(L"Depth Stencil");
	
}

void Renderer::InitModelPipeline()
{
	m_VertexShader->Create(L"Assets/Shaders/VS_GLTF.hlsl");
	m_PixelShader->Create(L"Assets/Shaders/PS_GLTF.hlsl");

	// TODO: Move input layouts to separate class
	std::array<D3D12_INPUT_ELEMENT_DESC, 5> layout{
		D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
		D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
		D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		D3D12_INPUT_ELEMENT_DESC{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		D3D12_INPUT_ELEMENT_DESC{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{ D3D_ROOT_SIGNATURE_VERSION_1_1 };
	if (FAILED(m_Device->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		// Rollback to 1_0
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	D3D12_ROOT_SIGNATURE_FLAGS rootFlags{};
	rootFlags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
	rootFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	rootFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	rootFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	std::array<CD3DX12_DESCRIPTOR_RANGE1, 3> ranges{};
	ranges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0);
	// Textures
	ranges.at(1).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	ranges.at(2).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	// Sampler
	//ranges.at(3).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	
	std::array<CD3DX12_ROOT_PARAMETER1, 3> params{};
	params.at(0).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	params.at(1).InitAsDescriptorTable(1, &ranges.at(1), D3D12_SHADER_VISIBILITY_PIXEL);
	params.at(2).InitAsDescriptorTable(1, &ranges.at(2), D3D12_SHADER_VISIBILITY_PIXEL);
	//params.at(3).InitAsDescriptorTable(1, &ranges.at(3), D3D12_SHADER_VISIBILITY_PIXEL);

	//D3D12_SAMPLER_DESC samplerDesc{};
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	//m_Device->GetDevice()->CreateSampler(&samplerDesc, )
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootDesc{};
	rootDesc.Init_1_1(static_cast<uint32_t>(params.size()), params.data(), 1, &samplerDesc, rootFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;

	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, signature.GetAddressOf(), error.GetAddressOf()));
	
	ThrowIfFailed(m_Device->GetDevice()->CreateRootSignature(0,
				  signature.Get()->GetBufferPointer(), signature.Get()->GetBufferSize(),
				  IID_PPV_ARGS(m_ModelRootSignature.GetAddressOf())));
	m_ModelRootSignature.Get()->SetName(L"ModelRootSignature");


	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = m_ModelRootSignature.Get();
	psoDesc.InputLayout = { layout.data(), static_cast<uint32_t>(layout.size()) };
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_VertexShader->GetData());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_PixelShader->GetData());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	ThrowIfFailed(m_Device->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_ModelPipelineState.GetAddressOf())));
	m_ModelPipelineState.Get()->SetName(L"ModelPipelineState");

}
