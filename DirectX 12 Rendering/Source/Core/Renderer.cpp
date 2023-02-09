#include "Renderer.hpp"
#include "../Utils/Utils.hpp"
#include <array>


Renderer::Renderer(HINSTANCE hInstance)
{
	m_Window = std::make_unique<Window>(hInstance);
}

Renderer::~Renderer()
{
	OnDestroy();
}

void Renderer::Initialize()
{
	m_Window->Initialize();
	auto hwnd = m_Window->GetHWND();
	Device::Initialize(hwnd);

	InitTriangle();
}

void Renderer::InitPipelineState()
{
}

void Renderer::Update()
{
}

void Renderer::Draw()
{
	//https://www.braynzarsoft.net/viewtutorial/q16390-03-initializing-directx-12

	RecordCommandLists();

	ComPtr<ID3D12CommandList> commandLists{ GetCommandList() };
	GetCommandQueue()->ExecuteCommandLists(1, commandLists.GetAddressOf());

	ThrowIfFailed(GetSwapChain()->Present(1, 0));

	WaitForPreviousFrame();
}

void Renderer::RecordCommandLists()
{
	ThrowIfFailed(GetCommandAllocator()->Reset());
	ThrowIfFailed(GetCommandList()->Reset(m_CommandAllocator.Get(), m_PipelineState.Get()));
	
	GetCommandList()->SetGraphicsRootSignature(m_RootSignature.Get());
	GetCommandList()->RSSetViewports(1, &m_Viewport);
	GetCommandList()->RSSetScissorRects(1, &m_ViewportRect);

	auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(),
														 D3D12_RESOURCE_STATE_PRESENT,
														 D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_CommandList.Get()->ResourceBarrier(1, &barrier1);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, GetDescriptorSize());
	m_CommandList.Get()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	std::array<const float, 4> clearColor{ 0.5f, 0.5f, 1.0f, 1.0f };
	m_CommandList.Get()->ClearRenderTargetView(rtvHandle, clearColor.data(), 0, nullptr);

	// Begin Render Triangle 
	GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	GetCommandList()->IASetVertexBuffers(0, 1, &m_VertexView);
	GetCommandList()->DrawInstanced(3, 1, 0, 0);

	// End Render Triangle

	auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(),
														 D3D12_RESOURCE_STATE_RENDER_TARGET,
														 D3D12_RESOURCE_STATE_PRESENT);
	GetCommandList()->ResourceBarrier(1, &barrier2);

	ThrowIfFailed(GetCommandList()->Close());

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

void Renderer::OnDestroy()
{
	WaitForPreviousFrame();

	SafeRelease(m_PipelineState);
	SafeRelease(m_VertexBuffer);
}

void Renderer::InitTriangle()
{
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

	// Triangle Shaders
	ComPtr<ID3DBlob> vsBlob, vsErr;
	ComPtr<ID3DBlob> psBlob, psErr;

	uint32_t compileFlags{ 0 };
#if defined (_DEBUG) || DEBUG
	compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	
	// TODO: Move to separate Shader class later
	HRESULT hr = D3DCompileFromFile(L"Assets/Shaders/VS_Triangle.hlsl", nullptr, nullptr, "VS", "vs_5_0", compileFlags, 0, vsBlob.GetAddressOf(), vsErr.GetAddressOf());
	if (FAILED(hr))
	{
		if (vsErr != nullptr)
		{
			OutputDebugStringA((char*)vsErr->GetBufferPointer());
		}
	}

	hr = D3DCompileFromFile(L"Assets/Shaders/PS_Triangle.hlsl", nullptr, nullptr, "PS", "ps_5_0", compileFlags, 0, psBlob.GetAddressOf(), psErr.GetAddressOf());
	if (FAILED(hr))
	{

	}
	
	std::array<D3D12_INPUT_ELEMENT_DESC, 2> layout{
		D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
		D3D12_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { layout.data(), static_cast<uint32_t>(layout.size()) };
	psoDesc.pRootSignature = m_RootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
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
	ThrowIfFailed(GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_PipelineState.GetAddressOf())));

	// CommandList for PipelineState
	Device::CreateCommandList(m_PipelineState.Get());

	//----
	std::array<TriangleVertex, 3 > vertices{
		TriangleVertex{ XMFLOAT3( 0.0f,  0.5f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		TriangleVertex{ XMFLOAT3( 0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		TriangleVertex{ XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
	};

	auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
	auto bufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)) };
	ThrowIfFailed(GetDevice()->CreateCommittedResource(&heapProperties,
				  D3D12_HEAP_FLAG_NONE,
				  &bufferDesc,
				  D3D12_RESOURCE_STATE_GENERIC_READ,
				  nullptr,
				  IID_PPV_ARGS(m_VertexBuffer.GetAddressOf())));

	uint8_t* pVertexDataBegin{};
	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(m_VertexBuffer.Get()->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, vertices.data(), static_cast<size_t>(sizeof(TriangleVertex) * vertices.size()));
	m_VertexBuffer.Get()->Unmap(0, nullptr);

	m_VertexView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VertexView.StrideInBytes = sizeof(TriangleVertex);
	m_VertexView.SizeInBytes = sizeof(vertices);

}
