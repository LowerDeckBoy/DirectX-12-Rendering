#include "ImageBasedLighting.hpp"
#include "../Core/DescriptorHeap.hpp"
#include "../Rendering/Camera.hpp"
#include "../Core/DeviceContext.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "TextureUtils.hpp"
#include <vector>


ImageBasedLighting::~ImageBasedLighting()
{
	Release();
}

void ImageBasedLighting::Create(DeviceContext* pDevice, const std::string_view& Filepath)
{
	InitializeTextures(pDevice, Filepath);
	InitializeBuffers(pDevice);

	assert(m_CommandList = pDevice->GetCommandList());
}

void ImageBasedLighting::InitializeTextures(DeviceContext* pDevice, const std::string_view& Filepath)
{
	ID3D12RootSignature* computeRoot{ nullptr };
	//D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC
	std::array<CD3DX12_DESCRIPTOR_RANGE1, 2> ranges{};
	ranges.at(0) = { D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE };
	ranges.at(1) = { D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE };

	std::array<CD3DX12_ROOT_PARAMETER1, 3> parameters{};
	parameters.at(0).InitAsDescriptorTable(1, &ranges.at(0));
	parameters.at(1).InitAsDescriptorTable(1, &ranges.at(1));
	parameters.at(2).InitAsConstants(1, 0);
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC signatureDesc{};
	const CD3DX12_STATIC_SAMPLER_DESC computeSamplerDesc{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
	signatureDesc.Init_1_1(static_cast<uint32_t>(parameters.size()), parameters.data(), 1, &computeSamplerDesc);

	const D3D12_ROOT_SIGNATURE_FLAGS standardFlags = D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;

	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error));
	ThrowIfFailed(pDevice->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&computeRoot)));
	computeRoot->SetName(L"Compute Root");

	m_EnvironmentTexture = TextureUtils::CreateResource(pDevice->GetDevice(), TextureDesc(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), TextureData(1024, 1024, 6, DXGI_FORMAT_R16G16B16A16_FLOAT));
	pDevice->GetMainHeap()->Allocate(m_EnvDescriptor);
	TextureUtils::CreateUAV(pDevice->GetDevice(), m_EnvironmentTexture.Get(), m_EnvDescriptor, DXGI_FORMAT_R16G16B16A16_FLOAT, 6);
	//TextureUtils::CreateUAV(pDevice->GetDevice(), m_EnvironmentTexture.Get(), m_EnvDescriptor, DXGI_FORMAT_R8G8B8A8_UNORM, 6);

	Shader eq2c("Assets/Shaders/Compute/Equirectangular2Cube.hlsl", "cs_5_1");

	ID3D12Resource* resource = TextureUtils::CreateFromHDR(pDevice, Filepath);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	pDevice->GetMainHeap()->Allocate(m_Descriptor);
	pDevice->GetDevice()->CreateShaderResourceView(resource, &srvDesc, m_Descriptor.GetCPU());

	ID3D12PipelineState* computePipeline;
	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = computeRoot;
	psoDesc.CS = { eq2c.GetData()->GetBufferPointer(), eq2c.GetData()->GetBufferSize() };
	pDevice->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&computePipeline));

	const auto barrier{ CD3DX12_RESOURCE_BARRIER::Transition(m_EnvironmentTexture.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS) };
	m_EnvironmentTexture.Get()->SetName(L"Environment Texture");

	const auto commandList{ pDevice->GetCommandList() };
	commandList->SetDescriptorHeaps(1, pDevice->GetMainHeap()->GetHeapAddressOf());
	commandList->ResourceBarrier(1, &barrier);
	commandList->SetComputeRootSignature(computeRoot);
	commandList->SetPipelineState(computePipeline);
	commandList->SetComputeRootDescriptorTable(0, m_Descriptor.GetGPU());
	commandList->SetComputeRootDescriptorTable(1, m_EnvDescriptor.GetGPU());
	commandList->Dispatch(1024 / 32, 1024 / 32, 6);

	m_OutputResource = TextureUtils::CreateResource(pDevice->GetDevice(), TextureDesc(), { 1024, 1024, 6, DXGI_FORMAT_R16G16B16A16_FLOAT });
	m_OutputResource.Get()->SetName(L"IBL Output Resource");

	std::array<D3D12_RESOURCE_BARRIER, 2> preCopyBarriers{};
	preCopyBarriers.at(0) = CD3DX12_RESOURCE_BARRIER::Transition(m_EnvironmentTexture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	preCopyBarriers.at(1) = CD3DX12_RESOURCE_BARRIER::Transition(m_OutputResource.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->ResourceBarrier(static_cast<uint32_t>(preCopyBarriers.size()), preCopyBarriers.data());

	commandList->CopyResource(m_OutputResource.Get(), m_EnvironmentTexture.Get());

	// States back to COMMON
	std::array<D3D12_RESOURCE_BARRIER, 2> postCopyBarriers{};
	postCopyBarriers.at(0) = CD3DX12_RESOURCE_BARRIER::Transition(m_EnvironmentTexture.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
	postCopyBarriers.at(1) = CD3DX12_RESOURCE_BARRIER::Transition(m_OutputResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
	commandList->ResourceBarrier(static_cast<uint32_t>(postCopyBarriers.size()), postCopyBarriers.data());

	//
	pDevice->GetMainHeap()->Allocate(m_OutputDescriptor);
	D3D12_SHADER_RESOURCE_VIEW_DESC cubeDesc{};
	cubeDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	cubeDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	//cubeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	cubeDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	cubeDesc.TextureCube.MipLevels = 1;
	pDevice->GetDevice()->CreateShaderResourceView(m_OutputResource.Get(), &cubeDesc, m_OutputDescriptor.GetCPU());

	PrefilterSpecular(pDevice, computeRoot);


}

void ImageBasedLighting::InitializeBuffers(DeviceContext* pDevice)
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

}

void ImageBasedLighting::CreateCubeTexture(DeviceContext* pDeviceCtx, const std::string_view& Filepath)
{
}

void ImageBasedLighting::PrefilterSpecular(DeviceContext* pDeviceCtx, ID3D12RootSignature* pComputeRoot)
{
	ID3D12PipelineState* pipelineState{ nullptr };
	Shader cs("Assets/Shaders/Compute/SpecularMap.hlsl", "cs_5_1");

	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = pComputeRoot;
	psoDesc.CS = CD3DX12_SHADER_BYTECODE(cs.GetData());
	ThrowIfFailed(pDeviceCtx->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));

	m_PrefilteredMap = TextureUtils::CreateResource(pDeviceCtx->GetDevice(), TextureDesc(), TextureData(1024, 1024, 6, DXGI_FORMAT_R16G16B16A16_FLOAT));

	const auto commandList{ pDeviceCtx->GetCommandList() };

	std::array<D3D12_RESOURCE_BARRIER, 2> preCopyBarriers{};
	preCopyBarriers.at(0) = CD3DX12_RESOURCE_BARRIER::Transition(m_EnvironmentTexture.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	preCopyBarriers.at(1) = CD3DX12_RESOURCE_BARRIER::Transition(m_PrefilteredMap.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(static_cast<uint32_t>(preCopyBarriers.size()), preCopyBarriers.data());

	commandList->SetDescriptorHeaps(1, pDeviceCtx->GetMainHeap()->GetHeapAddressOf());
	commandList->SetComputeRootSignature(pComputeRoot);
	commandList->SetPipelineState(pipelineState);
	commandList->SetComputeRootDescriptorTable(0, m_OutputDescriptor.GetGPU());
	commandList->SetComputeRootDescriptorTable(1, m_EnvDescriptor.GetGPU());
	commandList->Dispatch(1024 / 32, 1024 / 32, 6);

	preCopyBarriers.at(0) = CD3DX12_RESOURCE_BARRIER::Transition(m_EnvironmentTexture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	preCopyBarriers.at(1) = CD3DX12_RESOURCE_BARRIER::Transition(m_PrefilteredMap.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->ResourceBarrier(static_cast<uint32_t>(preCopyBarriers.size()), preCopyBarriers.data());

	commandList->CopyResource(m_PrefilteredMap.Get(), m_EnvironmentTexture.Get());

	std::array<D3D12_RESOURCE_BARRIER, 2> postCopyBarriers{};
	postCopyBarriers.at(0) = CD3DX12_RESOURCE_BARRIER::Transition(m_EnvironmentTexture.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_GENERIC_READ);
	postCopyBarriers.at(1) = CD3DX12_RESOURCE_BARRIER::Transition(m_PrefilteredMap.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandList->ResourceBarrier(static_cast<uint32_t>(postCopyBarriers.size()), postCopyBarriers.data());

	pDeviceCtx->GetMainHeap()->Allocate(m_PrefilteredDescriptor);
	D3D12_SHADER_RESOURCE_VIEW_DESC cubeDesc{};
	cubeDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//cubeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	cubeDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	cubeDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	cubeDesc.TextureCube.MipLevels = 1;
	pDeviceCtx->GetDevice()->CreateShaderResourceView(m_PrefilteredMap.Get(), &cubeDesc, m_PrefilteredDescriptor.GetCPU());

}

void ImageBasedLighting::Draw(Camera* pCamera, uint32_t FrameIndex)
{
	m_CommandList.Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_CommandList.Get()->IASetVertexBuffers(0, 1, &m_VertexBuffer.View);
	m_CommandList.Get()->IASetIndexBuffer(&m_IndexBuffer.View);

	UpdateWorld(pCamera);
	m_ConstBuffer.Update({ XMMatrixTranspose(m_WorldMatrix * pCamera->GetViewProjection()),
								XMMatrixTranspose(XMMatrixIdentity()) }, FrameIndex);
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
	SAFE_RELEASE(m_PrefilteredMap);
	SAFE_RELEASE(m_EnvironmentTexture);
	SAFE_RELEASE(m_OutputResource);
	SAFE_RELEASE(m_CommandList);

}
