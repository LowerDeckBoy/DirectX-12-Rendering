#include "ImageBasedLighting.hpp"
#include "../Core/DescriptorHeap.hpp"
#include "../Rendering/Camera.hpp"
#include "../Core/DeviceContext.hpp"
#include "Shader.hpp"
//#include "Texture.hpp"
#include "TextureUtils.hpp"
#include <vector>


ImageBasedLighting::~ImageBasedLighting()
{
	Release();
}

void ImageBasedLighting::Create(DeviceContext* pDeviceCtx, const std::string_view& Filepath)
{
	CreateTextures(pDeviceCtx, Filepath);
	CreateBuffers(pDeviceCtx);

	assert(m_CommandList = pDeviceCtx->GetCommandList());
}

void ImageBasedLighting::CreateTextures(DeviceContext* pDeviceCtx, const std::string_view& Filepath)
{
	ID3D12RootSignature* computeRootSignature{ nullptr };
	// Compute Root Signature
	{
		std::array<CD3DX12_DESCRIPTOR_RANGE1, 2> ranges{};
		ranges.at(0) = { D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC };
		ranges.at(1) = { D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE };

		std::array<CD3DX12_ROOT_PARAMETER1, 3> parameters{};
		// SRV
		parameters.at(0).InitAsDescriptorTable(1, &ranges.at(0));
		// UAV
		parameters.at(1).InitAsDescriptorTable(1, &ranges.at(1));
		parameters.at(2).InitAsConstants(1, 0);
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC signatureDesc{};
		const CD3DX12_STATIC_SAMPLER_DESC computeSamplerDesc{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
		signatureDesc.Init_1_1(static_cast<uint32_t>(parameters.size()), parameters.data(), 1, &computeSamplerDesc);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;

		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error));
		ThrowIfFailed(pDeviceCtx->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&computeRootSignature)));
		computeRootSignature->SetName(L"[Image Based Lighting] Compute Root Signature");
	}

	CreateCubeTexture(pDeviceCtx, computeRootSignature, Filepath);

	// Execute Command List BEFORE creating SRV of given Resource
	// otherwise it will cause issues when creating next SRVs
	CreateIrradiance(pDeviceCtx, computeRootSignature);
	CreateSpecular(pDeviceCtx, computeRootSignature);
	CreateSpecularBRDF(pDeviceCtx, computeRootSignature);

	SAFE_DELETE(computeRootSignature)
}

void ImageBasedLighting::CreateBuffers(DeviceContext* pDevice)
{
	std::vector<SkyboxVertex>* vertices = new std::vector<SkyboxVertex>{
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) }
	};

	std::vector<uint32_t>* indices = new std::vector<uint32_t>{
		0, 1, 2, 0, 2, 3,
		4, 6, 5, 4, 7, 6,
		4, 5, 1, 4, 1, 0,
		3, 2, 6, 3, 6, 7,
		1, 5, 6, 1, 6, 2,
		4, 0, 3, 4, 3, 7
	};

	m_VertexBuffer.Create(pDevice, BufferData(vertices->data(), vertices->size(), vertices->size() * sizeof(vertices->at(0)), sizeof(vertices->at(0))), BufferDesc());
	m_IndexBuffer.Create(pDevice, BufferData(indices->data(), indices->size(), indices->size() * sizeof(indices->at(0)), sizeof(indices->at(0))), BufferDesc());

	m_ConstBuffer.Create(pDevice, &m_cbData);

	delete vertices;
	delete indices;
}

void ImageBasedLighting::CreateCubeTexture(DeviceContext* pDeviceCtx, ID3D12RootSignature* pComputeRoot, const std::string_view& Filepath)
{
	ID3D12Resource* environmentTexture{ nullptr };
	environmentTexture = TextureUtils::CreateResource(pDeviceCtx->GetDevice(), 
		TextureDesc(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), 
		TextureData(1024, 1024, 6, DXGI_FORMAT_R16G16B16A16_FLOAT));

	// Temporal Descriptor to hold pretransformed texture
	Descriptor uavDescriptor;
	pDeviceCtx->GetMainHeap()->Allocate(uavDescriptor);
	TextureUtils::CreateUAV(pDeviceCtx->GetDevice(), environmentTexture, uavDescriptor, DXGI_FORMAT_R16G16B16A16_FLOAT, 6);

	Shader eq2c("Assets/Shaders/Compute/Equirectangular2Cube.hlsl", "cs_5_1");

	ID3D12Resource* resource = TextureUtils::CreateFromHDR(pDeviceCtx, Filepath);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// Temporal Descriptor
	Descriptor envDescriptor;
	pDeviceCtx->GetMainHeap()->Allocate(envDescriptor);
	pDeviceCtx->GetDevice()->CreateShaderResourceView(resource, &srvDesc, envDescriptor.GetCPU());

	ID3D12PipelineState* computePipeline;
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
		psoDesc.pRootSignature = pComputeRoot;
		psoDesc.CS = { eq2c.GetData()->GetBufferPointer(), eq2c.GetData()->GetBufferSize() };
		pDeviceCtx->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&computePipeline));
	}

	const auto barrier{ CD3DX12_RESOURCE_BARRIER::Transition(environmentTexture, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS) };
	environmentTexture->SetName(L"[Image Based Lighting] Environment Texture - pretransformed");

	const auto commandList{ pDeviceCtx->GetCommandList() };

	auto dispatch = [&](ID3D12GraphicsCommandList4* pCommandList) {
			commandList->SetDescriptorHeaps(1, pDeviceCtx->GetMainHeap()->GetHeapAddressOf());
			commandList->ResourceBarrier(1, &barrier);
			commandList->SetComputeRootSignature(pComputeRoot);
			commandList->SetPipelineState(computePipeline);
			commandList->SetComputeRootDescriptorTable(0, envDescriptor.GetGPU());
			commandList->SetComputeRootDescriptorTable(1, uavDescriptor.GetGPU());
			commandList->Dispatch(1024 / 32, 1024 / 32, 6);
		};
	dispatch(commandList);
	
	//commandList->Dispatch(1024 / 32, 1024 / 32, 6);

	m_OutputResource = TextureUtils::CreateResource(pDeviceCtx->GetDevice(), TextureDesc(), { 1024, 1024, 6, DXGI_FORMAT_R16G16B16A16_FLOAT });
	m_OutputResource.Get()->SetName(L"[Image Based Lighting] TextureCube Resource");

	std::array<D3D12_RESOURCE_BARRIER, 2> preCopyBarriers{};
	preCopyBarriers.at(0) = CD3DX12_RESOURCE_BARRIER::Transition(environmentTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	preCopyBarriers.at(1) = CD3DX12_RESOURCE_BARRIER::Transition(m_OutputResource.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->ResourceBarrier(static_cast<uint32_t>(preCopyBarriers.size()), preCopyBarriers.data());

	commandList->CopyResource(m_OutputResource.Get(), environmentTexture);

	// States back to COMMON
	std::array<D3D12_RESOURCE_BARRIER, 2> postCopyBarriers{};
	postCopyBarriers.at(0) = CD3DX12_RESOURCE_BARRIER::Transition(environmentTexture, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
	postCopyBarriers.at(1) = CD3DX12_RESOURCE_BARRIER::Transition(m_OutputResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
	commandList->ResourceBarrier(static_cast<uint32_t>(postCopyBarriers.size()), postCopyBarriers.data());

	pDeviceCtx->ExecuteCommandList(true);

	pDeviceCtx->GetMainHeap()->Allocate(m_OutputDescriptor);
	srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	pDeviceCtx->GetDevice()->CreateShaderResourceView(m_OutputResource.Get(), &srvDesc, m_OutputDescriptor.GetCPU());

	SAFE_DELETE(computePipeline);
	SAFE_DELETE(environmentTexture);
	SAFE_DELETE(resource);

}

void ImageBasedLighting::CreateIrradiance(DeviceContext* pDeviceCtx, ID3D12RootSignature* pComputeRoot)
{
	ID3D12PipelineState* pipelineState{ nullptr };
	// Pipeline
	{
		Shader cs("Assets/Shaders/Compute/IrradianceMap.hlsl", "cs_5_1");

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
		psoDesc.pRootSignature = pComputeRoot;
		psoDesc.CS = CD3DX12_SHADER_BYTECODE(cs.GetData());
		ThrowIfFailed(pDeviceCtx->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
	}

	// Resource and Descriptor creation
	{
		m_IrradianceMap = TextureUtils::CreateResource(pDeviceCtx->GetDevice(),
			TextureDesc(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
			TextureData(32, 32, 6, DXGI_FORMAT_R16G16B16A16_FLOAT));

		pDeviceCtx->GetMainHeap()->Allocate(m_IrradianceDescriptor);
		TextureUtils::CreateUAV(pDeviceCtx->GetDevice(), m_IrradianceMap.Get(), m_IrradianceDescriptor, DXGI_FORMAT_R16G16B16A16_FLOAT, 6);

		m_IrradianceMap.Get()->SetName(L"[Image Based Lighting] Irradiance Map");
	}

	// Compute execution and resource transition
	const auto dispatch = [&](ID3D12GraphicsCommandList4* pCommandList) {
		pCommandList->SetDescriptorHeaps(1, pDeviceCtx->GetMainHeap()->GetHeapAddressOf());
		pCommandList->SetComputeRootSignature(pComputeRoot);
		pCommandList->SetPipelineState(pipelineState);
		pCommandList->SetComputeRootDescriptorTable(0, m_OutputDescriptor.GetGPU());
		pCommandList->SetComputeRootDescriptorTable(1, m_IrradianceDescriptor.GetGPU());
		pCommandList->Dispatch(32 / 32, 32 / 32, 6);

		const auto toCommon = CD3DX12_RESOURCE_BARRIER::Transition(m_IrradianceMap.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
		pCommandList->ResourceBarrier(1, &toCommon);
		};
	dispatch(pDeviceCtx->GetCommandList());

	

	pDeviceCtx->ExecuteCommandList(true);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	pDeviceCtx->GetDevice()->CreateShaderResourceView(m_IrradianceMap.Get(), &srvDesc, m_IrradianceDescriptor.GetCPU());

	SAFE_DELETE(pipelineState);

}

void ImageBasedLighting::CreateSpecular(DeviceContext* pDeviceCtx, ID3D12RootSignature* pComputeRoot)
{
	ID3D12PipelineState* pipelineState{ nullptr };
	// Pipeline
	{
		Shader cs("Assets/Shaders/Compute/SpecularMap.hlsl", "cs_5_1");

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
		psoDesc.pRootSignature = pComputeRoot;
		psoDesc.CS = CD3DX12_SHADER_BYTECODE(cs.GetData());
		ThrowIfFailed(pDeviceCtx->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
	}

	// Resource and Descriptor creation
	{
		m_SpecularMap = TextureUtils::CreateResource(pDeviceCtx->GetDevice(),
			TextureDesc(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
			TextureData(256, 256, 6, DXGI_FORMAT_R16G16B16A16_FLOAT));

		pDeviceCtx->GetMainHeap()->Allocate(m_SpecularDescriptor);
		TextureUtils::CreateUAV(pDeviceCtx->GetDevice(), m_SpecularMap.Get(), m_SpecularDescriptor, DXGI_FORMAT_R16G16B16A16_FLOAT, 6);

		m_SpecularMap.Get()->SetName(L"[Image Based Lighting] Specular Map");
	}

	//https://github.com/Nadrin/PBR/blob/master/src/d3d12.cpp#L445
	// Compute execution and resource transition
	const auto dispatch = [&](ID3D12GraphicsCommandList4* pCommandList) {
		pCommandList->SetDescriptorHeaps(1, pDeviceCtx->GetMainHeap()->GetHeapAddressOf());
		pCommandList->SetComputeRootSignature(pComputeRoot);
		pCommandList->SetPipelineState(pipelineState);
		pCommandList->SetComputeRootDescriptorTable(0, m_OutputDescriptor.GetGPU());
		pCommandList->SetComputeRootDescriptorTable(1, m_SpecularDescriptor.GetGPU());
		pCommandList->Dispatch(256 / 32, 256 / 32, 6);

		const auto toCommon = CD3DX12_RESOURCE_BARRIER::Transition(m_SpecularMap.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
		pCommandList->ResourceBarrier(1, &toCommon);
		};
	dispatch(pDeviceCtx->GetCommandList());
	
	pDeviceCtx->ExecuteCommandList(true);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	pDeviceCtx->GetDevice()->CreateShaderResourceView(m_SpecularMap.Get(), &srvDesc, m_SpecularDescriptor.GetCPU());

	SAFE_DELETE(pipelineState);

}

void ImageBasedLighting::CreateSpecularBRDF(DeviceContext* pDeviceCtx, ID3D12RootSignature* pComputeRoot)
{
	ID3D12PipelineState* pipelineState{ nullptr };
	// Pipeline
	{
		Shader cs("Assets/Shaders/Compute/SpBRDF_LUT.hlsl", "cs_5_1");

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
		psoDesc.pRootSignature = pComputeRoot;
		psoDesc.CS = CD3DX12_SHADER_BYTECODE(cs.GetData());
		ThrowIfFailed(pDeviceCtx->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
	}

	// Resource and Descriptor creation
	{
		m_SpecularBRDF_LUT = TextureUtils::CreateResource(pDeviceCtx->GetDevice(),
			TextureDesc(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
			TextureData(256, 256, 1, DXGI_FORMAT_R16G16_FLOAT));

		pDeviceCtx->GetMainHeap()->Allocate(m_SpBRDFDescriptor);
		TextureUtils::CreateUAV(pDeviceCtx->GetDevice(), m_SpecularBRDF_LUT.Get(), m_SpBRDFDescriptor, DXGI_FORMAT_R16G16_FLOAT, 1);

		m_SpecularBRDF_LUT.Get()->SetName(L"[Image Based Lighting] Specular BRDF LUT");
	}

	// Compute execution and resource transition
	const auto dispatch = [&](ID3D12GraphicsCommandList4* pCommandList) {
		pCommandList->SetDescriptorHeaps(1, pDeviceCtx->GetMainHeap()->GetHeapAddressOf());
		pCommandList->SetComputeRootSignature(pComputeRoot);
		pCommandList->SetPipelineState(pipelineState);
		pCommandList->SetComputeRootDescriptorTable(1, m_SpBRDFDescriptor.GetGPU());
		pCommandList->Dispatch(256 / 32, 256 / 32, 6);

		const auto toGeneric{ CD3DX12_RESOURCE_BARRIER::Transition(m_SpecularBRDF_LUT.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ) };
		pCommandList->ResourceBarrier(1, &toGeneric);
		};
	dispatch(pDeviceCtx->GetCommandList());

	pDeviceCtx->ExecuteCommandList(true);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	pDeviceCtx->GetDevice()->CreateShaderResourceView(m_SpecularBRDF_LUT.Get(), &srvDesc, m_SpBRDFDescriptor.GetCPU());

	SAFE_DELETE(pipelineState);

}

void ImageBasedLighting::Draw(Camera* pCamera, uint32_t FrameIndex)
{
	m_CommandList.Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_CommandList.Get()->IASetVertexBuffers(0, 1, &m_VertexBuffer.View);
	m_CommandList.Get()->IASetIndexBuffer(&m_IndexBuffer.View);

	UpdateWorld(pCamera);
	m_ConstBuffer.Update({ XMMatrixTranspose(m_WorldMatrix * pCamera->GetViewProjection()), XMMatrixTranspose(XMMatrixIdentity()) }, FrameIndex);
	m_CommandList.Get()->SetGraphicsRootConstantBufferView(0, m_ConstBuffer.GetBuffer(FrameIndex)->GetGPUVirtualAddress());
	m_CommandList.Get()->SetGraphicsRootDescriptorTable(1, m_OutputDescriptor.GetGPU());

	m_CommandList.Get()->DrawIndexedInstanced(m_IndexBuffer.Count, 1, 0, 0, 0);
}

void ImageBasedLighting::UpdateWorld(Camera* pCamera)
{
	m_WorldMatrix = XMMatrixIdentity();
	m_Translation = XMVectorSet(DirectX::XMVectorGetX(pCamera->GetPosition()),
		DirectX::XMVectorGetY(pCamera->GetPosition()),
		DirectX::XMVectorGetZ(pCamera->GetPosition()), 0.0f);

	m_WorldMatrix = XMMatrixScalingFromVector(m_Scale) * XMMatrixTranslationFromVector(m_Translation);
}

void ImageBasedLighting::Release()
{
	SAFE_RELEASE(m_IrradianceMap);
	SAFE_RELEASE(m_SpecularBRDF_LUT);
	SAFE_RELEASE(m_SpecularMap);
	SAFE_RELEASE(m_OutputResource);
	SAFE_RELEASE(m_CommandList);
}
