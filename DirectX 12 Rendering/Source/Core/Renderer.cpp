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

	CreateDepthStencil();

	InitTriangle();

	//LoadAssets("Assets/Textures/jak_tam.jpg");
	LoadAssets("Assets/Textures/shrek.jpg");
	//LoadAssets("Assets/Textures/pointers.jpg");
	
}

void Renderer::InitPipelineState()
{
}

void Renderer::Update(const XMMATRIX ViewProj)
{
	// Updating const buffer
	//const float speed{ 0.1f };
	//const float offsetBounds{ 1.25f };
	//
	//m_cbData.Offset.x += speed;
	//if (m_cbData.Offset.x > offsetBounds)
	//{
	//	m_cbData.Offset.x = -offsetBounds;
	//}
	//std::memcpy(m_ConstBuffer.pDataBegin, &m_cbData, sizeof(m_cbData));

	const XMMATRIX world{ XMMatrixIdentity() };
	m_cbPerObject.WVP = world * ViewProj;
	//m_cbPerObject.WVP *= ViewProj;
	std::memcpy(m_ConstBuffer.pDataBegin, &m_cbPerObject, sizeof(m_cbPerObject));

}

void Renderer::Draw()
{
	RecordCommandLists();

	ID3D12CommandList* commandLists[]{ m_Device->GetCommandList() };
	m_Device->GetCommandQueue()->ExecuteCommandLists(_countof(commandLists), commandLists);

	//ThrowIfFailed(m_Device->GetSwapChain()->Present(1, 0));
	HRESULT hResult{ m_Device->GetSwapChain()->Present(1, 0) };
	if (hResult == DXGI_ERROR_DEVICE_REMOVED || hResult == DXGI_ERROR_DEVICE_RESET)
	{
		OutputDebugStringA("PRESENT FAIL!\n");
	}

	MoveToNextFrame();
	//WaitForGPU();
	//WaitForPreviousFrame();
}

void Renderer::RecordCommandLists()
{
	ThrowIfFailed(m_Device->m_CommandAllocators[m_Device->m_FrameIndex]->Reset());
	ThrowIfFailed(m_Device->GetCommandList()->Reset(m_Device->m_CommandAllocators[m_Device->m_FrameIndex].Get(), m_PipelineState.Get()));
	
	m_GUI->Begin();

	m_Device->GetCommandList()->SetGraphicsRootSignature(m_Device->m_RootSignature.Get());
	//, 
	// 
	ID3D12DescriptorHeap* ppHeaps[] = { m_Device->m_cbvHeap.Get() };
	m_Device->GetCommandList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	//m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(0, m_Device->m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	//m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(1, m_Device->m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	m_Device->GetCommandList()->RSSetViewports(1, &m_Device->m_Viewport);
	m_Device->GetCommandList()->RSSetScissorRects(1, &m_Device->m_ViewportRect);

	m_Device->GetCommandList()->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &m_cbPerObject.WVP, 0);

	auto presentToRender = CD3DX12_RESOURCE_BARRIER::Transition(m_Device->m_RenderTargets[m_Device->m_FrameIndex].Get(),
														 D3D12_RESOURCE_STATE_PRESENT,
														 D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_Device->GetCommandList()->ResourceBarrier(1, &presentToRender);

	// RTV and Depth
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_Device->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(), m_Device->m_FrameIndex, m_Device->GetDescriptorSize());
	CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle{ GetDepthHeap()->GetCPUDescriptorHandleForHeapStart() };
	m_Device->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &depthHandle);

	m_Device->GetCommandList()->ClearRenderTargetView(rtvHandle, m_ClearColor.data(), 0, nullptr);
	m_Device->GetCommandList()->ClearDepthStencilView(GetDepthHeap()->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// Begin Render Triangle 
	m_Device->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Device->GetCommandList()->IASetVertexBuffers(0, 1, &m_VertexBuffer.GetBufferView());
	m_Device->GetCommandList()->IASetIndexBuffer(&m_IndexBuffer.GetBufferView());
	m_Device->GetCommandList()->DrawIndexedInstanced(m_IndexBuffer.GetSize(), 1, 0, 0, 0);
	// End Render Triangle

	m_GUI->End(m_Device->GetCommandList());

	auto renderToPresent = CD3DX12_RESOURCE_BARRIER::Transition(m_Device->m_RenderTargets[m_Device->m_FrameIndex].Get(),
														 D3D12_RESOURCE_STATE_RENDER_TARGET,
														 D3D12_RESOURCE_STATE_PRESENT);
	m_Device->GetCommandList()->ResourceBarrier(1, &renderToPresent);


	ThrowIfFailed(m_Device->GetCommandList()->Close());

}

/*
void Renderer::WaitForPreviousFrame()
{
	const uint64_t currentValue = m_Device->m_FenceValue;
	ThrowIfFailed(m_Device->GetCommandQueue()->Signal(m_Device->GetFence(), currentValue));
	m_Device->m_FenceValue++;

	if (m_Device->GetFence()->GetCompletedValue() < currentValue)
	{
		ThrowIfFailed(m_Device->GetFence()->SetEventOnCompletion(currentValue, m_Device->m_FenceEvent));

		WaitForSingleObject(m_Device->m_FenceEvent, INFINITE);
	}

	m_Device->m_FrameIndex = m_Device->GetSwapChain()->GetCurrentBackBufferIndex();
}
*/

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
	m_Device->GetCommandList()->Reset(m_Device->m_CommandAllocators[m_Device->m_FrameIndex].Get(), nullptr);

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

	SafeRelease(m_PipelineState);
	//SafeRelease(m_VertexBuffer);
}

void Renderer::InitTriangle()
{
	/*
	D3D12_ROOT_SIGNATURE_DESC desc{};
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	desc.NumParameters = 0;
	desc.NumStaticSamplers = 0;
	desc.pParameters = nullptr;
	desc.pStaticSamplers = nullptr;

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, signature.GetAddressOf(), error.GetAddressOf()));
	ThrowIfFailed(GetDevice()->CreateRootSignature(0, signature.Get()->GetBufferPointer(), signature.Get()->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
	*/

	// Triangle Shaders
	//m_VertexShader->Create(L"Assets/Shaders/VS_Texture.hlsl");
	//m_PixelShader->Create(L"Assets/Shaders/PS_Texture.hlsl");

	//m_VertexShader->Create(L"Assets/Shaders/VS_CB.hlsl");
	//m_PixelShader->Create(L"Assets/Shaders/PS_CB.hlsl");
	m_VertexShader->Create(L"Assets/Shaders/TEST.hlsl");
	m_PixelShader->Create(L"Assets/Shaders/TEST.hlsl");

	//D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	std::array<D3D12_INPUT_ELEMENT_DESC, 2> layout{
		D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
		D3D12_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
	};

	// Root Signature
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{ D3D_ROOT_SIGNATURE_VERSION_1_1 };
	// If VERSION_1_1 is not supported rollback to 1_0
	if (FAILED(m_Device->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags{};
	rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
	rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;


	//std::array<CD3DX12_DESCRIPTOR_RANGE1, 1> ranges{};
	//ranges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	std::array<CD3DX12_DESCRIPTOR_RANGE1, 1> ranges{};
	//ranges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

	//std::array<CD3DX12_ROOT_PARAMETER1, 1> rootParameters{};
	//rootParameters.at(0).InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
	std::array<CD3DX12_ROOT_PARAMETER1, 1> rootParameters{};
	//rootParameters.at(0).InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters.at(0).InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	//rootParameters.at(1).InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);

	// Sampler
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Init_1_1(static_cast<uint32_t>(rootParameters.size()), rootParameters.data(), 1, &samplerDesc, rootSignatureFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, 
				  D3D_ROOT_SIGNATURE_VERSION_1_1, 
				  signature.GetAddressOf(), error.GetAddressOf()));

	ThrowIfFailed(m_Device->GetDevice()->CreateRootSignature(0, 
				  signature.Get()->GetBufferPointer(), 
				  signature.Get()->GetBufferSize(), 
				  IID_PPV_ARGS(m_Device->m_RootSignature.GetAddressOf())));

	SafeRelease(signature);
	SafeRelease(error);

	// PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { layout.data(), static_cast<uint32_t>(layout.size()) };
	psoDesc.pRootSignature = m_Device->m_RootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_VertexShader->GetData());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_PixelShader->GetData());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(m_Device->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_PipelineState.GetAddressOf())));

	// CommandList for PipelineState
	m_Device->CreateCommandList(m_PipelineState.Get());

	//----
	/*
	std::array<TriangleVertex, 3 > vertices{
		TriangleVertex{ XMFLOAT3( 0.0f,  0.5f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		TriangleVertex{ XMFLOAT3( 0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		TriangleVertex{ XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
	};
	*/

	/*
	std::vector<VertexUV> vertices{
		{ XMFLOAT3(0.0f,  0.5f, 0.0f),   XMFLOAT2(0.5f, 0.0f) },
		{ XMFLOAT3(0.5f, -0.5f, 0.0f),   XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-0.5f, -0.5f, 0.0f),  XMFLOAT2(0.0f, 1.0f) }
	};*/

	// Buffers begin
	/*
	std::vector<VertexUV> vertices{
		{ XMFLOAT3( 0.5f,  0.5f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3( 0.5f, -0.5f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-0.5f,  0.5f, 0.0f), XMFLOAT2(0.0f, 0.0f) }
	};*/

	std::vector<CubeVertex> vertices{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) }, 
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3( 1.0f,  1.0f, -1.0f),  XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f),  XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3( 1.0f,  1.0f,  1.0f),  XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3( 1.0f, -1.0f,  1.0f),  XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) }
	};

	m_VertexBuffer.Create(m_Device.get(), vertices);

	/*
	std::vector<uint32_t> indices{
		0, 1, 2,
		2, 3, 0,
	};
	*/
	std::vector<uint32_t> indices{

	0, 1, 2, 0, 2, 3,

	4, 6, 5, 4, 7, 6,

	4, 5, 1, 4, 1, 0,

	3, 2, 6, 3, 6, 7,

	1, 5, 6, 1, 6, 2,

	4, 0, 3, 4, 3, 7
	};

	m_IndexBuffer.Create(m_Device.get(), indices);


	// Const buffer
	m_ConstBuffer.Create(m_Device.get(), &m_cbPerObject);
	//m_ConstBuffer.Create(m_Device.get(), &m_cbData);
	

	// Buffers end

}

void Renderer::LoadAssets(const std::string& TexturePath)
{
	std::wstring wpath = std::wstring(TexturePath.begin(), TexturePath.end());
	const wchar_t* path = wpath.c_str();

	// Getting texture info
	DirectX::ScratchImage scratchImage{};
	DirectX::LoadFromWICFile(path, WIC_FLAGS_FORCE_RGB, 
							 nullptr, 
							 scratchImage);
	DirectX::TexMetadata metadata{ scratchImage.GetMetadata() };

	D3D12_RESOURCE_DESC textureDesc{};
	textureDesc.Format = metadata.format;
	textureDesc.Width = static_cast<uint32_t>(metadata.width);
	textureDesc.Height = static_cast<uint32_t>(metadata.height);
	textureDesc.MipLevels = 1;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.Alignment = 0;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	//D3D12_RESOURCE_STATE_COPY_DEST
	ComPtr<ID3D12Resource> texture;
	auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
	ThrowIfFailed(m_Device->GetDevice()->CreateCommittedResource(&heapProperties,
				  D3D12_HEAP_FLAG_NONE, &textureDesc, 
				  D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
				  IID_PPV_ARGS(m_TriangleTexture.GetAddressOf())));

	const uint64_t bufferSize{ GetRequiredIntermediateSize(m_TriangleTexture.Get(), 0, 1) };

	
	// GPU upload
	auto heapUploadProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
	auto bufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize) };
	ThrowIfFailed(m_Device->GetDevice()->CreateCommittedResource(&heapUploadProperties, D3D12_HEAP_FLAG_NONE,
				  &bufferDesc,
				  D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
				  IID_PPV_ARGS(&texture)));

	D3D12_SUBRESOURCE_DATA subresource{};
	subresource.pData = scratchImage.GetPixels();
	subresource.RowPitch = scratchImage.GetImages()->rowPitch;
	subresource.SlicePitch = scratchImage.GetImages()->slicePitch;

	UpdateSubresources(m_Device->GetCommandList(), m_TriangleTexture.Get(), texture.Get(), 0, 0, 1, &subresource);
	auto copyToResource{ CD3DX12_RESOURCE_BARRIER::Transition(m_TriangleTexture.Get(), 
															  D3D12_RESOURCE_STATE_COPY_DEST, 
															  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) };
	m_Device->GetCommandList()->ResourceBarrier(1, &copyToResource);
	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	
	auto srvCPUHandle{ m_Device->m_srvHeap.Get()->GetCPUDescriptorHandleForHeapStart() };
	m_Device->GetDevice()->CreateShaderResourceView(m_TriangleTexture.Get(), &srvDesc, srvCPUHandle);

	// Due to ComPtrs being used on CPU 
	// ComPtr objects need to be uploaded to GPU BEFORE
	// going out of scope

	// GPU pre-execution
	// required to ensure that texture data is inside GPU memory
	// before ComPtr is released, therefore destroyed

	//WaitForGPU();
	ThrowIfFailed(m_Device->GetCommandList()->Close());
	ID3D12CommandList* ppCommandLists[] = { m_Device->GetCommandList() };
	m_Device->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//WaitForPreviousFrame();
	WaitForGPU();

}

void Renderer::CreateDepthStencil()
{
	/*
	D3D12_DEPTH_STENCIL_DESC dsDesc{};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	dsDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	dsDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	dsDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	*/
	D3D12_DESCRIPTOR_HEAP_DESC dsHeap{};
	dsHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsHeap.NumDescriptors = 1;
	ThrowIfFailed(m_Device->GetDevice()->CreateDescriptorHeap(&dsHeap, IID_PPV_ARGS(m_DepthHeap.GetAddressOf())));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsView{};
	dsView.Flags = D3D12_DSV_FLAG_NONE;
	dsView.Format = DXGI_FORMAT_D32_FLOAT;
	dsView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;
	//clearValue.Color[0] = m_ClearColor.at(0);
	//clearValue.Color[1] = m_ClearColor.at(1);
	//clearValue.Color[2] = m_ClearColor.at(2);
	//clearValue.Color[3] = m_ClearColor.at(3);

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
	m_DepthHeap.Get()->SetName(L"DepthBuffer");

	auto cpuHandle{ m_DepthHeap.Get()->GetCPUDescriptorHandleForHeapStart() };
	m_Device->GetDevice()->CreateDepthStencilView(m_DepthStencil.Get(), &dsView, cpuHandle);

}
