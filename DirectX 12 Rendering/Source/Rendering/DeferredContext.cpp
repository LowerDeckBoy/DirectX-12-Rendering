#include "Camera.hpp"
#include "../Core/DescriptorHeap.hpp"
#include "../Core/DeviceContext.hpp"
#include "../Graphics/ShaderManager.hpp"
#include "DeferredContext.hpp"
#include "../Core/GraphicsPipelineState.hpp"
#include "Model/Model.hpp"
#include "../Utilities/Utilities.hpp"
#include <imgui.h>


DeferredContext::DeferredContext()
{
}

DeferredContext::DeferredContext(DeviceContext* pDeviceContext, ShaderManager* pShaderManager, ID3D12RootSignature* pModelRootSignature)
{
	Create(pDeviceContext, pShaderManager, pModelRootSignature);
}

DeferredContext::~DeferredContext()
{
}

void DeferredContext::Create(DeviceContext* pDeviceContext, ShaderManager* pShaderManager, ID3D12RootSignature* pModelRootSignature)
{
	assert(m_DeviceCtx = pDeviceContext);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = DeferredRTVCount;
	m_DeviceCtx->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_DeferredHeap.ReleaseAndGetAddressOf()));

	m_ScreenQuad = std::make_unique<ScreenQuad>(pDeviceContext);

	CreateRenderTargets();
	CreatePipelines(pShaderManager, pModelRootSignature);

}

void DeferredContext::OnResize()
{
	CreateRenderTargets();
}

void DeferredContext::PassGBuffer(Camera* pCamera, ConstantBuffer<SceneConstData>* CameraCB, std::vector<Model*>& Models)
{
	for (uint32_t i = 0; i < DeferredRTVCount; i++)
	{
		const auto toRender{ CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets.at(i).Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET)};
		m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &toRender);
	}

	//clear
	for (uint32_t i = 0; i < DeferredRTVCount; i++)
		m_DeviceCtx->GetCommandList()->ClearRenderTargetView(m_RenderTargetDescriptors.at(i), m_ClearColor.data(), 0, nullptr);

	m_DeviceCtx->GetCommandList()->SetPipelineState(m_PipelineState.Get());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootConstantBufferView(1, CameraCB->GetBuffer(m_DeviceCtx->FRAME_INDEX)->GetGPUVirtualAddress());

	const CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle(m_DeviceCtx->GetDepthHeap()->GetCPUDescriptorHandleForHeapStart());
	m_DeviceCtx->GetCommandList()->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_DeviceCtx->GetCommandList()->OMSetRenderTargets(DeferredRTVCount, m_RenderTargetDescriptors.data(), false, &depthHandle);
	
	for (const auto& model : Models)
		model->Draw(pCamera);

	for (uint32_t i = 0; i < DeferredRTVCount; i++)
	{
		const auto toRead{ CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets.at(i).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ) };
		m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &toRead);
	}
}

void DeferredContext::PassLight(Camera* pCamera)
{
	const auto depthWriteToRead{ CD3DX12_RESOURCE_BARRIER::Transition(m_DeviceCtx->GetDepthStencil(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ) };
	m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &depthWriteToRead);

	// Set PSO and Root for deferred
	m_DeviceCtx->GetCommandList()->SetPipelineState(m_LightPipelineState.Get());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootSignature(m_RootSignature.Get());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(0, m_ShaderDescriptors.at(0).GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(1, m_ShaderDescriptors.at(1).GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(2, m_ShaderDescriptors.at(2).GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(3, m_ShaderDescriptors.at(3).GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(4, m_ShaderDescriptors.at(4).GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(5, m_DeviceCtx->GetDepthDescriptor().GetGPU());
	//m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(6, m_Skybox->GetTex().m_Descriptor.GetGPU());
	//m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(6, m_IBL->m_OutputDescriptor.GetGPU());

	//m_DeviceCtx->GetCommandList()->SetGraphicsRootConstantBufferView(7, m_cbCamera->GetBuffer(m_DeviceCtx->FRAME_INDEX)->GetGPUVirtualAddress());
	//m_DeviceCtx->GetCommandList()->SetGraphicsRootConstantBufferView(8, m_PointLights->m_cbPointLights->GetBuffer(m_DeviceCtx->FRAME_INDEX)->GetGPUVirtualAddress());

	m_ScreenQuad->Draw(m_DeviceCtx->GetCommandList());

	const auto depthReadToWrite{ CD3DX12_RESOURCE_BARRIER::Transition(m_DeviceCtx->GetDepthStencil(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE) };
	m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &depthReadToWrite);
}

void DeferredContext::DrawDeferredTargets()
{

	ImGui::Begin("Render Targets");
	const auto rtv0 = m_ShaderDescriptors.at(0).GetGPU();
	const auto rtv1 = m_ShaderDescriptors.at(1).GetGPU();
	const auto rtv2 = m_ShaderDescriptors.at(2).GetGPU();
	const auto rtv3 = m_ShaderDescriptors.at(3).GetGPU();
	const auto rtv4 = m_ShaderDescriptors.at(4).GetGPU();
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
}

void DeferredContext::CreateRenderTargets()
{
	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.MipLevels = 1;
	desc.DepthOrArraySize = 1;
	desc.Width = static_cast<uint64_t>(m_DeviceCtx->GetViewport().Width);
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

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_DeferredHeap->GetCPUDescriptorHandleForHeapStart());
	const auto heapProps{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
	for (uint32_t i = 0; i < DeferredRTVCount; i++)
	{
		desc.Format  = m_RTVFormats.at(i);
		clear.Format = m_RTVFormats.at(i);
		ThrowIfFailed(m_DeviceCtx->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, &clear, IID_PPV_ARGS(m_RenderTargets.at(i).ReleaseAndGetAddressOf())));

		std::wstring wname{ L"Deferred Render Target #" + std::to_wstring(i) };
		m_RenderTargets.at(i).Get()->SetName(wname.c_str());
		//LPCWSTR name{ wname.c_str() };
		//m_RenderTargets.at(i).Get()->SetName(name);

		// Try to allocate descriptors only once
		// RTVs
		m_RenderTargetDescriptors.at(i) = rtvHandle;
		m_DeviceCtx->GetDevice()->CreateRenderTargetView(m_RenderTargets.at(i).Get(), &rtvDesc, m_RenderTargetDescriptors.at(i));
		rtvHandle.Offset(1, 32);

		// SRVs
		m_DeviceCtx->GetMainHeap()->Allocate(m_ShaderDescriptors.at(i));
		m_DeviceCtx->GetDevice()->CreateShaderResourceView(m_RenderTargets.at(i).Get(), &srvDesc, m_ShaderDescriptors.at(i).GetCPU());
	}

}

void DeferredContext::CreatePipelines(ShaderManager* pShaderManager, ID3D12RootSignature* pModelRootSignature)
{
	// Root Signature
	{
		std::vector<CD3DX12_DESCRIPTOR_RANGE1> deferredRanges(7);
		deferredRanges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(1).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(2).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(3).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(4).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(5).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(6).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

		std::vector<CD3DX12_ROOT_PARAMETER1> deferredParams(9);
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
		// CBVs
		// Camera Data
		deferredParams.at(7).InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
		// Light Data
		deferredParams.at(8).InitAsConstantBufferView(1, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

		PSOBuilder* builder{ new PSOBuilder() };

		builder->AddRanges(deferredRanges);
		builder->AddParameters(deferredParams);
		builder->AddSampler(0);
		builder->CreateRootSignature(m_DeviceCtx->GetDevice(), m_RootSignature.GetAddressOf(), L"Deferred Root Signature");

		builder->Reset();

		delete builder;
	}
	
	// PSOs
	{
		// GBuffer PSO

		auto layout{ PSOUtils::CreateInputLayout() };

		IDxcBlob* const deferredVertex = pShaderManager->CreateDXIL("Assets/Shaders/Deferred/GBuffer_Vertex.hlsl", L"vs_6_0");
		IDxcBlob* const deferredPixel = pShaderManager->CreateDXIL("Assets/Shaders/Deferred/GBuffer_Pixel.hlsl", L"ps_6_0");
		//
		D3D12_GRAPHICS_PIPELINE_STATE_DESC deferredDesc{};
		deferredDesc.pRootSignature = pModelRootSignature;
		deferredDesc.InputLayout = { layout.data(), static_cast<uint32_t>(layout.size()) };
		deferredDesc.VS = CD3DX12_SHADER_BYTECODE(deferredVertex->GetBufferPointer(), deferredVertex->GetBufferSize());
		deferredDesc.PS = CD3DX12_SHADER_BYTECODE(deferredPixel->GetBufferPointer(), deferredPixel->GetBufferSize());
		deferredDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		deferredDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		deferredDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		deferredDesc.SampleMask = UINT_MAX;
		deferredDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		deferredDesc.NumRenderTargets = DeferredRTVCount;
		deferredDesc.RTVFormats[0] = m_RTVFormats.at(0);
		deferredDesc.RTVFormats[1] = m_RTVFormats.at(1);
		deferredDesc.RTVFormats[2] = m_RTVFormats.at(2);
		deferredDesc.RTVFormats[3] = m_RTVFormats.at(3);
		deferredDesc.RTVFormats[4] = m_RTVFormats.at(4);
		deferredDesc.RTVFormats[5] = m_RTVFormats.at(5);
		deferredDesc.DSVFormat = m_DeviceCtx->GetDepthFormat();
		deferredDesc.SampleDesc.Count = 1;

		m_DeviceCtx->GetDevice()->CreateGraphicsPipelineState(&deferredDesc, IID_PPV_ARGS(m_PipelineState.ReleaseAndGetAddressOf()));
		m_PipelineState.Get()->SetName(L"GBuffer Pipeline State");

		// Light Pass PSO
		auto screenQuadLayout{ PSOUtils::CreateScreenQuadLayout() };

		IDxcBlob* const quadVertex = pShaderManager->CreateDXIL("Assets/Shaders/Deferred/Deferred_Vertex.hlsl", L"vs_6_0");
		IDxcBlob* const quadPixel = pShaderManager->CreateDXIL("Assets/Shaders/Deferred/Deferred_Pixel.hlsl", L"ps_6_0");

		D3D12_GRAPHICS_PIPELINE_STATE_DESC lightDesc{};
		lightDesc.pRootSignature = m_RootSignature.Get();
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

		m_DeviceCtx->GetDevice()->CreateGraphicsPipelineState(&lightDesc, IID_PPV_ARGS(m_LightPipelineState.ReleaseAndGetAddressOf()));
		m_LightPipelineState.Get()->SetName(L"Light Pass Pipeline State");
	}
	
}
