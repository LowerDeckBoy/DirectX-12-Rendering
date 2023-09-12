#include "Camera.hpp"
#include "../Core/DescriptorHeap.hpp"
#include "../Core/DeviceContext.hpp"
#include "../Graphics/ShaderManager.hpp"
#include "DeferredContext.hpp"
#include "../Graphics/ImageBasedLighting.hpp"
#include "Light/PointLights.hpp"
#include "../Core/GraphicsPipelineState.hpp"
#include "Model/Model.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Utilities/Logger.hpp"
//#include "../Core/Window.hpp"
#include <imgui.h>


DeferredContext::DeferredContext(DeviceContext* pDeviceContext, ShaderManager* pShaderManager, ID3D12RootSignature* pModelRootSignature)
{
	Create(pDeviceContext, pShaderManager, pModelRootSignature);
}

DeferredContext::~DeferredContext()
{
	Release();
}

void DeferredContext::Create(DeviceContext* pDeviceContext, ShaderManager* pShaderManager, ID3D12RootSignature* pModelRootSignature)
{
	assert(m_DeviceCtx = pDeviceContext);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = RenderTargetsCount;
	m_DeviceCtx->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_DeferredHeap.ReleaseAndGetAddressOf()));
	m_DeferredHeap.Get()->SetName(L"Deferred Render Target Heap");

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDesc.NumDescriptors = 1;
	m_DeviceCtx->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_DeferredDepthHeap.ReleaseAndGetAddressOf()));
	m_DeferredHeap.Get()->SetName(L"Deferred Depth Heap");

	ScreenQuad::Create(pDeviceContext);

	CreatePipelines(pShaderManager, pModelRootSignature);

	CreateRenderTargets();
	CreateShadowMap();

}

void DeferredContext::OnResize()
{
	CreateRenderTargets();
	CreateShadowMap();
}

void DeferredContext::PassGBuffer(Camera* pCamera, ConstantBuffer<SceneConstData>* CameraCB, std::vector<std::unique_ptr<Model>>& Models)
{
	for (uint32_t i = 0; i < RenderTargetsCount; i++)
	{
		const auto toRender{ CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets.at(i).Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET)};
		m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &toRender);
	}

	for (uint32_t i = 0; i < RenderTargetsCount; i++)
		m_DeviceCtx->GetCommandList()->ClearRenderTargetView(m_RenderTargetDescriptors.at(i), m_ClearColor.data(), 0, nullptr);

	m_DeviceCtx->GetCommandList()->SetPipelineState(m_PipelineState.Get());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootConstantBufferView(1, CameraCB->GetBuffer(m_DeviceCtx->FRAME_INDEX)->GetGPUVirtualAddress());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(6, m_DeviceCtx->GetDepthDescriptor().GetGPU());

	const CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle(m_DeviceCtx->GetDepthHeap()->GetCPUDescriptorHandleForHeapStart());
	m_DeviceCtx->GetCommandList()->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_DeviceCtx->GetCommandList()->OMSetRenderTargets(RenderTargetsCount, m_RenderTargetDescriptors.data(), false, &depthHandle);
	
	for (const auto& model : Models)
		model->Draw(pCamera);

	for (uint32_t i = 0; i < RenderTargetsCount; i++)
	{
		const auto toRead{ CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets.at(i).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ) };
		m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &toRead);
	}
}

void DeferredContext::PassLight(Camera* pCamera, ConstantBuffer<SceneConstData>* CameraCB, ImageBasedLighting* pIBL, PointLights* pPointLights)
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

	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(6, pIBL->m_OutputDescriptor.GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(7, pIBL->m_IrradianceDescriptor.GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(8, pIBL->m_SpecularDescriptor.GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(9, pIBL->m_SpBRDFDescriptor.GetGPU());

	m_DeviceCtx->GetCommandList()->SetGraphicsRootConstantBufferView(10, CameraCB->GetBuffer(m_DeviceCtx->FRAME_INDEX)->GetGPUVirtualAddress());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootConstantBufferView(11, pPointLights->m_cbPointLights->GetBuffer(m_DeviceCtx->FRAME_INDEX)->GetGPUVirtualAddress());

	//test
	ScreenQuad::Draw(m_DeviceCtx->GetCommandList());

	const auto depthReadToWrite{ CD3DX12_RESOURCE_BARRIER::Transition(m_DeviceCtx->GetDepthStencil(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE) };
	m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &depthReadToWrite);

}

void DeferredContext::PassShadows(Camera* pCamera, ConstantBuffer<SceneConstData>* CameraCB, PointLights* pPointLights, ImageBasedLighting* pIBL, std::vector<std::unique_ptr<Model>>& Models)
{
	// TODO: Add private helper enums for root signature params across whole section
	const auto shadowMapWriteToRead{ CD3DX12_RESOURCE_BARRIER::Transition(m_ShadowMap.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_READ)};
	m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &shadowMapWriteToRead);

	const auto depthWriteToRead{ CD3DX12_RESOURCE_BARRIER::Transition(m_DeviceCtx->GetDepthStencil(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ) };
	m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &depthWriteToRead);

	m_DeviceCtx->GetCommandList()->SetPipelineState(m_ShadowPipelineState.Get());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootSignature(m_ShadowRootSignature.Get());

	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(0, m_ShaderDescriptors.at(0).GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(1, m_ShaderDescriptors.at(1).GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(2, m_ShaderDescriptors.at(2).GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(3, m_ShaderDescriptors.at(3).GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(4, m_ShaderDescriptors.at(4).GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(5, m_DeviceCtx->GetDepthDescriptor().GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(6, m_ShadowMapDescriptor.GetGPU());	
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(7, pIBL->m_OutputDescriptor.GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(8, pIBL->m_IrradianceDescriptor.GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(9, pIBL->m_SpecularDescriptor.GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootDescriptorTable(10, pIBL->m_SpBRDFDescriptor.GetGPU());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootConstantBufferView(11, CameraCB->GetBuffer(m_DeviceCtx->FRAME_INDEX)->GetGPUVirtualAddress());
	m_DeviceCtx->GetCommandList()->SetGraphicsRootConstantBufferView(12, pPointLights->m_cbLightData->GetBuffer(m_DeviceCtx->FRAME_INDEX)->GetGPUVirtualAddress());

	//const auto& depth = m_ShadowMapDescriptor.GetGPU().ptr;
	//ImGui::Image(reinterpret_cast<ImTextureID>(depth), { 800, 600 });

	ScreenQuad::Draw(m_DeviceCtx->GetCommandList());

	const auto shadowMapReadToWrite{ CD3DX12_RESOURCE_BARRIER::Transition(m_ShadowMap.Get(), D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_GENERIC_READ) };
	m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &shadowMapReadToWrite);
	const auto depthReadToWrite{ CD3DX12_RESOURCE_BARRIER::Transition(m_DeviceCtx->GetDepthStencil(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE) };
	m_DeviceCtx->GetCommandList()->ResourceBarrier(1, &depthReadToWrite);
}

void DeferredContext::DrawDeferredTargets()
{
	ImGui::Begin("Render Targets");

	const auto& baseColor		{ m_ShaderDescriptors.at(0).GetGPU().ptr };
	const auto& normal			{ m_ShaderDescriptors.at(1).GetGPU().ptr };
	const auto& metalRoughness	{ m_ShaderDescriptors.at(2).GetGPU().ptr };
	const auto& emissive		{ m_ShaderDescriptors.at(3).GetGPU().ptr };
	const auto& worldPosition	{ m_ShaderDescriptors.at(4).GetGPU().ptr };
	const auto& depth			{ m_ShaderDescriptors.at(5).GetGPU().ptr };
	//const auto& depth = m_DeviceCtx->GetDepthDescriptor().GetGPU().ptr;
	//const auto& depth = m_ShadowMapDescriptor.GetGPU().ptr;

	ImGui::Image(reinterpret_cast<ImTextureID>(depth),			{ 500, 350 });
	ImGui::SameLine();
	ImGui::Image(reinterpret_cast<ImTextureID>(baseColor),		{ 500, 350 });
	ImGui::SameLine();
	ImGui::Image(reinterpret_cast<ImTextureID>(normal),			{ 500, 350 });
	ImGui::Image(reinterpret_cast<ImTextureID>(metalRoughness), { 500, 350 });
	ImGui::SameLine();
	ImGui::Image(reinterpret_cast<ImTextureID>(emissive),		{ 500, 350 });
	ImGui::SameLine();
	ImGui::Image(reinterpret_cast<ImTextureID>(worldPosition),	{ 500, 350 });
	//ImGui::SameLine();
	//if (m_DeviceCtx->GetDepthDescriptor().bIsValid())

	ImGui::End();
}

void DeferredContext::CreateRenderTargets()
{
	if (m_RenderTargets.at(0).Get())
	{
		for (auto& target : m_RenderTargets)
			SAFE_RELEASE(target);
	}

	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.MipLevels = 1;
	desc.DepthOrArraySize = 1;
	desc.Width  = static_cast<uint64_t>(m_DeviceCtx->GetViewport().Width);
	desc.Height = static_cast<uint32_t>(m_DeviceCtx->GetViewport().Height);
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags  = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
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
	for (uint32_t i = 0; i < RenderTargetsCount; i++)
	{
		desc.Format  = m_RenderTargetFormats.at(i);
		clear.Format = m_RenderTargetFormats.at(i);
		ThrowIfFailed(m_DeviceCtx->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, &clear, IID_PPV_ARGS(m_RenderTargets.at(i).ReleaseAndGetAddressOf())));

		const std::wstring wname{ L"Deferred Render Target #" + std::to_wstring(i) };
		m_RenderTargets.at(i).Get()->SetName(wname.c_str());

		m_RenderTargetDescriptors.at(i) = rtvHandle;
		m_DeviceCtx->GetDevice()->CreateRenderTargetView(m_RenderTargets.at(i).Get(), &rtvDesc, m_RenderTargetDescriptors.at(i));
		rtvHandle.Offset(1, 32);

		// SRVs
		m_DeviceCtx->GetMainHeap()->Allocate(m_ShaderDescriptors.at(i));
		m_DeviceCtx->GetDevice()->CreateShaderResourceView(m_RenderTargets.at(i).Get(), &srvDesc, m_ShaderDescriptors.at(i).GetCPU());
	}

	m_DeviceCtx->ExecuteCommandList(true);

}

void DeferredContext::CreatePipelines(ShaderManager* pShaderManager, ID3D12RootSignature* pModelRootSignature)
{
	PSOBuilder* builder{ new PSOBuilder() };

	// Root Signature
	{
		std::vector<CD3DX12_DESCRIPTOR_RANGE1> deferredRanges(10);
		deferredRanges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(1).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(2).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(3).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(4).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(5).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(6).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(7).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(8).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		deferredRanges.at(9).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

		std::vector<CD3DX12_ROOT_PARAMETER1> deferredParams(12);
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
		// Image Based Lighting - Skybox
		deferredParams.at(7).InitAsDescriptorTable(1, &deferredRanges.at(7), D3D12_SHADER_VISIBILITY_PIXEL);
		// Irradiance
		deferredParams.at(8).InitAsDescriptorTable(1, &deferredRanges.at(8), D3D12_SHADER_VISIBILITY_PIXEL);
		// Specular
		deferredParams.at(9).InitAsDescriptorTable(1, &deferredRanges.at(9), D3D12_SHADER_VISIBILITY_PIXEL);
		// CBVs
		deferredParams.at(10).InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
		// Light Data
		deferredParams.at(11).InitAsConstantBufferView(1, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

		builder->AddRanges(deferredRanges);
		builder->AddParameters(deferredParams);
		builder->AddSampler(0);
		builder->AddSampler(1, 0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS);
		builder->CreateRootSignature(m_DeviceCtx->GetDevice(), m_RootSignature.GetAddressOf(), L"Deferred Root Signature");

		builder->Reset();

		deferredParams.clear();
		deferredRanges.clear();
	}

	// Shadows Root Signature
	{
		std::vector<CD3DX12_DESCRIPTOR_RANGE1> shadowRanges(11);
		shadowRanges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		shadowRanges.at(1).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		shadowRanges.at(2).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		shadowRanges.at(3).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		shadowRanges.at(4).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		shadowRanges.at(5).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		shadowRanges.at(6).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		shadowRanges.at(7).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		shadowRanges.at(8).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		shadowRanges.at(9).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
		shadowRanges.at(10).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 10, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

		std::vector<CD3DX12_ROOT_PARAMETER1> shadowParams(13);
		// GBuffer output textures
		// Base Color
		shadowParams.at(0).InitAsDescriptorTable(1, &shadowRanges.at(0), D3D12_SHADER_VISIBILITY_PIXEL);
		// Normal
		shadowParams.at(1).InitAsDescriptorTable(1, &shadowRanges.at(1), D3D12_SHADER_VISIBILITY_PIXEL);
		// Metallic
		shadowParams.at(2).InitAsDescriptorTable(1, &shadowRanges.at(2), D3D12_SHADER_VISIBILITY_PIXEL);
		// Emissive
		shadowParams.at(3).InitAsDescriptorTable(1, &shadowRanges.at(3), D3D12_SHADER_VISIBILITY_PIXEL);
		// Positions
		shadowParams.at(4).InitAsDescriptorTable(1, &shadowRanges.at(4), D3D12_SHADER_VISIBILITY_PIXEL);
		// Scene Depth
		shadowParams.at(5).InitAsDescriptorTable(1, &shadowRanges.at(5), D3D12_SHADER_VISIBILITY_PIXEL);
		// Shadow Map
		shadowParams.at(6).InitAsDescriptorTable(1, &shadowRanges.at(6), D3D12_SHADER_VISIBILITY_PIXEL);
		// Skybox
		shadowParams.at(7).InitAsDescriptorTable(1, &shadowRanges.at(7), D3D12_SHADER_VISIBILITY_PIXEL);
		// Image Based Lighting
		shadowParams.at(8).InitAsDescriptorTable(1, &shadowRanges.at(8), D3D12_SHADER_VISIBILITY_PIXEL);
		shadowParams.at(9).InitAsDescriptorTable(1, &shadowRanges.at(9), D3D12_SHADER_VISIBILITY_PIXEL);
		shadowParams.at(10).InitAsDescriptorTable(1, &shadowRanges.at(10), D3D12_SHADER_VISIBILITY_PIXEL);
		// CBVs
		shadowParams.at(11).InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
		shadowParams.at(12).InitAsConstantBufferView(1, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

		builder->AddRanges(shadowRanges);
		builder->AddParameters(shadowParams);
		builder->AddSampler(0);
		builder->AddSampler(1, 0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS);
		//builder->AddSampler(0);
		//builder->CreateRootSignature(m_DeviceCtx->GetDevice(), m_DeferredRootSignature.GetAddressOf(), L"Deferred Root Signature");

		// Light Matrices for Shadows
		//CD3DX12_ROOT_PARAMETER1 param{};
		//param.InitAsConstantBufferView(2, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
		//shadowParams.push_back(param);
		builder->CreateRootSignature(m_DeviceCtx->GetDevice(), m_ShadowRootSignature.ReleaseAndGetAddressOf(), L"Shadows Root Signature");

		//builder->Reset();
		shadowRanges.clear();
		shadowParams.clear();
	}
	
	builder->Reset();
	delete builder;
	
	// PSOs
	{
		// GBuffer PSO

		auto layout{ PSOUtils::CreateInputLayout() };

		IDxcBlob* const deferredVertex = pShaderManager->CreateDXIL("Assets/Shaders/Deferred/GBuffer_Vertex.hlsl", ShaderType::eVertex);
		IDxcBlob* const deferredPixel  = pShaderManager->CreateDXIL("Assets/Shaders/Deferred/GBuffer_Pixel.hlsl", ShaderType::ePixel);
		//
		D3D12_GRAPHICS_PIPELINE_STATE_DESC deferredDesc{};
		deferredDesc.pRootSignature = pModelRootSignature;
		deferredDesc.VS = CD3DX12_SHADER_BYTECODE(deferredVertex->GetBufferPointer(), deferredVertex->GetBufferSize());
		deferredDesc.PS = CD3DX12_SHADER_BYTECODE(deferredPixel->GetBufferPointer(), deferredPixel->GetBufferSize());
		deferredDesc.InputLayout = { layout.data(), static_cast<uint32_t>(layout.size()) };
		deferredDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		deferredDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		deferredDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		deferredDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		deferredDesc.SampleMask = UINT_MAX;
		deferredDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		deferredDesc.NumRenderTargets = RenderTargetsCount;
		deferredDesc.RTVFormats[0] = m_RenderTargetFormats.at(0);
		deferredDesc.RTVFormats[1] = m_RenderTargetFormats.at(1);
		deferredDesc.RTVFormats[2] = m_RenderTargetFormats.at(2);
		deferredDesc.RTVFormats[3] = m_RenderTargetFormats.at(3);
		deferredDesc.RTVFormats[4] = m_RenderTargetFormats.at(4);
		deferredDesc.RTVFormats[5] = m_RenderTargetFormats.at(5);
		deferredDesc.DSVFormat = m_DeviceCtx->GetDepthFormat();
		deferredDesc.SampleDesc.Count = 1;

		m_DeviceCtx->GetDevice()->CreateGraphicsPipelineState(&deferredDesc, IID_PPV_ARGS(m_PipelineState.ReleaseAndGetAddressOf()));
		m_PipelineState.Get()->SetName(L"GBuffer Pipeline State");

		// Light Pass PSO
		auto screenQuadLayout{ PSOUtils::CreateScreenQuadLayout() };

		IDxcBlob* const quadVertex = pShaderManager->CreateDXIL("Assets/Shaders/Deferred/Deferred_Vertex.hlsl", ShaderType::eVertex);
		IDxcBlob* const quadPixel  = pShaderManager->CreateDXIL("Assets/Shaders/Deferred/Deferred_Pixel.hlsl", ShaderType::ePixel);

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
		lightDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		lightDesc.RasterizerState.AntialiasedLineEnable = TRUE;
		lightDesc.SampleMask = UINT_MAX;
		lightDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		lightDesc.NumRenderTargets = 1;
		lightDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		lightDesc.DSVFormat = m_DeviceCtx->GetDepthFormat();
		lightDesc.SampleDesc = { 1, 0 };

		m_DeviceCtx->GetDevice()->CreateGraphicsPipelineState(&lightDesc, IID_PPV_ARGS(m_LightPipelineState.ReleaseAndGetAddressOf()));
		m_LightPipelineState.Get()->SetName(L"Light Pass Pipeline State");
	}

	// Pre-shadow PSO
	{
		IDxcBlob* const shadowsVertex = pShaderManager->CreateDXIL("Assets/Shaders/Deferred/Shadows_Clip_VS.hlsl", ShaderType::eVertex);
		IDxcBlob* const shadowsPixel  = pShaderManager->CreateDXIL("Assets/Shaders/Deferred/Shadows_Clip_PS.hlsl", ShaderType::ePixel);

		auto modelLayout{ PSOUtils::CreateInputLayout() };

		D3D12_GRAPHICS_PIPELINE_STATE_DESC preShadowDesc{};
		preShadowDesc.pRootSignature = pModelRootSignature;
		preShadowDesc.VS = CD3DX12_SHADER_BYTECODE(shadowsVertex->GetBufferPointer(), shadowsVertex->GetBufferSize());
		preShadowDesc.PS = CD3DX12_SHADER_BYTECODE(shadowsPixel->GetBufferPointer(), shadowsPixel->GetBufferSize());
		preShadowDesc.InputLayout = { modelLayout.data(), static_cast<uint32_t>(modelLayout.size()) };
		preShadowDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		preShadowDesc.DepthStencilState.DepthEnable = TRUE;
		preShadowDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		preShadowDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		preShadowDesc.RasterizerState.DepthClipEnable = TRUE;
		preShadowDesc.RasterizerState.DepthBias = 10000;
		preShadowDesc.RasterizerState.DepthBiasClamp = 0.0f;
		preShadowDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
		preShadowDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		preShadowDesc.SampleMask = UINT_MAX;
		preShadowDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		preShadowDesc.NumRenderTargets = 0;
		preShadowDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		preShadowDesc.DSVFormat = m_DeviceCtx->GetDepthFormat();
		preShadowDesc.SampleDesc = { 1, 0 };

		m_DeviceCtx->GetDevice()->CreateGraphicsPipelineState(&preShadowDesc, IID_PPV_ARGS(m_PreShadowPipelineState.ReleaseAndGetAddressOf()));
		m_PreShadowPipelineState.Get()->SetName(L"Pre-Shadow Pass Pipeline State");
	}

	// Shadows PSO
	{
		IDxcBlob* const shadowsVertex = pShaderManager->CreateDXIL("Assets/Shaders/Deferred/Simple_Shadows_VS.hlsl", ShaderType::eVertex);
		IDxcBlob* const shadowsPixel = pShaderManager->CreateDXIL("Assets/Shaders/Deferred/Simple_Shadows_PS.hlsl", ShaderType::ePixel);

		auto screenQuadLayout{ PSOUtils::CreateScreenQuadLayout() };

		D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowDesc{};
		//shadowDesc.pRootSignature = m_RootSignature.Get();
		shadowDesc.pRootSignature = m_ShadowRootSignature.Get();
		shadowDesc.VS = CD3DX12_SHADER_BYTECODE(shadowsVertex->GetBufferPointer(), shadowsVertex->GetBufferSize());
		shadowDesc.PS = CD3DX12_SHADER_BYTECODE(shadowsPixel->GetBufferPointer(), shadowsPixel->GetBufferSize());
		shadowDesc.InputLayout = { screenQuadLayout.data(), static_cast<uint32_t>(screenQuadLayout.size()) };
		shadowDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		shadowDesc.DepthStencilState.DepthEnable = false;
		shadowDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		shadowDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		shadowDesc.RasterizerState.DepthClipEnable = false;
		shadowDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		shadowDesc.SampleMask = UINT_MAX;
		shadowDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		shadowDesc.NumRenderTargets = 1;
		shadowDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		shadowDesc.DSVFormat = m_DeviceCtx->GetDepthFormat();
		shadowDesc.SampleDesc = { 1, 0 };

		m_DeviceCtx->GetDevice()->CreateGraphicsPipelineState(&shadowDesc, IID_PPV_ARGS(m_ShadowPipelineState.ReleaseAndGetAddressOf()));
		m_ShadowPipelineState.Get()->SetName(L"Shadow Pass Pipeline State");
	}
	
	m_DeviceCtx->ExecuteCommandList(true);

}

void DeferredContext::Release()
{
	SAFE_RELEASE(m_ShadowPipelineState);
	SAFE_RELEASE(m_PreShadowPipelineState);
	SAFE_RELEASE(m_PipelineState);
	SAFE_RELEASE(m_LightPipelineState);
	SAFE_RELEASE(m_RootSignature);

	for (auto& target : m_RenderTargets)
		SAFE_RELEASE(target);

	SAFE_RELEASE(m_DeferredDepthHeap);
	SAFE_RELEASE(m_DeferredHeap);

	Logger::Log("DeferredContext released.");
}

void DeferredContext::CreateShadowMap()
{
	if (m_ShadowMap)
		SAFE_RELEASE(m_ShadowMap);

	D3D12_RESOURCE_DESC desc{};
	desc.Width	= 2048;
	desc.Height = 2048;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Format = DXGI_FORMAT_R32_TYPELESS;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	desc.SampleDesc = { 1, 0 };

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = D3D12_MAX_DEPTH;
	clearValue.DepthStencil.Stencil = 0;

	const auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };

	ThrowIfFailed(m_DeviceCtx->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&clearValue,
		IID_PPV_ARGS(m_ShadowMap.ReleaseAndGetAddressOf())));
	m_ShadowMap.Get()->SetName(L"Shadow Map Resource");

	//https://www.gamedev.net/forums/topic/701041-shadow-mapping-with-directx-12/
	D3D12_DEPTH_STENCIL_VIEW_DESC dsView{};
	dsView.Flags = D3D12_DSV_FLAG_NONE;
	dsView.Format = DXGI_FORMAT_D32_FLOAT;
	dsView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsView.Texture2D.MipSlice = 0;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	m_DeviceCtx->GetDevice()->CreateDepthStencilView(m_ShadowMap.Get(), &dsView, m_DeferredDepthHeap.Get()->GetCPUDescriptorHandleForHeapStart());

	m_DeviceCtx->GetMainHeap()->Allocate(m_ShadowMapDescriptor);
	m_DeviceCtx->GetDevice()->CreateShaderResourceView(m_ShadowMap.Get(), &srvDesc, m_ShadowMapDescriptor.GetCPU());

	m_ShadowScrissor.left		= 0L;
	m_ShadowScrissor.top		= 0L;
	m_ShadowScrissor.right		= 2048;
	m_ShadowScrissor.bottom		= 2048;

	m_ShadowViewport.TopLeftX	= 0.0f;
	m_ShadowViewport.TopLeftY	= 0.0f;
	m_ShadowViewport.Width		= static_cast<float>(m_ShadowScrissor.right);
	m_ShadowViewport.Height		= static_cast<float>(m_ShadowScrissor.bottom);
	m_ShadowViewport.MinDepth	= D3D12_MIN_DEPTH;
	m_ShadowViewport.MaxDepth	= D3D12_MAX_DEPTH;
	
}

void DeferredContext::DrawToShadowMap(Camera* pCamera, ConstantBuffer<SceneConstData>* CameraCB, PointLights* pPointLights, std::vector<std::unique_ptr<Model>>& Models, ID3D12RootSignature* pModelRootSignature)
{
	const auto commandList{ m_DeviceCtx->GetCommandList() };

	const auto toDepthWrite{ CD3DX12_RESOURCE_BARRIER::Transition(m_ShadowMap.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE) };
	commandList->ResourceBarrier(1, &toDepthWrite);

	commandList->RSSetViewports(1, &m_ShadowViewport);
	commandList->RSSetScissorRects(1, &m_ShadowScrissor);

	// Writing to DepthBuffer only
	auto handle{ m_DeferredDepthHeap.Get()->GetCPUDescriptorHandleForHeapStart() };
	commandList->ClearDepthStencilView(m_DeferredDepthHeap.Get()->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	commandList->OMSetRenderTargets(0, nullptr, FALSE, &handle);

	commandList->SetPipelineState(m_PreShadowPipelineState.Get());
	commandList->SetGraphicsRootSignature(pModelRootSignature);
	commandList->SetGraphicsRootConstantBufferView(3, pPointLights->m_cbLightData->GetBuffer(m_DeviceCtx->FRAME_INDEX)->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(6, m_ShadowMapDescriptor.GetGPU());

	for (const auto& model : Models)
		model->Draw(pCamera);

	const auto toGenericRead{ CD3DX12_RESOURCE_BARRIER::Transition(m_ShadowMap.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ) };
	commandList->ResourceBarrier(1, &toGenericRead);

}
