#include "ImageBasedLighting.hpp"
#include "../Core/ComputePipelineState.hpp"
#include "../Core/ComputePipelineState.hpp"


void ImageBasedLighting::Create(Device* pDevice)
{
	//m_EnvironmentMap.Create(pDevice, "Assets/Textures/newport_loft.hdr");
	
	ComPtr<ID3D12RootSignature> computeRootSignature;

	ID3D12DescriptorHeap* computeDescriptorHeaps[] = {
		pDevice->m_cbvDescriptorHeap.GetHeap()
	};

	// Create universal compute root signature.
	{
		std::array<CD3DX12_DESCRIPTOR_RANGE1, 2> descriptorRanges{};
		descriptorRanges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		descriptorRanges.at(1).Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE);

		std::array<CD3DX12_ROOT_PARAMETER1, 3> rootParameters{};
		rootParameters.at(0).InitAsDescriptorTable(1, &descriptorRanges[0]);
		rootParameters.at(1).InitAsDescriptorTable(1, &descriptorRanges[1]);
		rootParameters.at(2).InitAsConstants(1, 0);

		auto sampler{ GraphicsPipelineState::CreateStaticSampler(0) };
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC signatureDesc{};
		signatureDesc.Init_1_1(static_cast<uint32_t>(rootParameters.size()), rootParameters.data(), 1, &sampler);
		computeRootSignature = ComputePipelineState::CreateRootSignature(pDevice->GetDevice(), descriptorRanges, rootParameters);
	}

	{
		Texture equirectangular(pDevice, "Assets/Textures/newport_loft.hdr");
		//equirectanglura.CreateUAV(equirectanglura.GetTexture(), 0);
		ComPtr<ID3D12PipelineState> pipelineState;
		Shader equiToCubeShader("Assets/Shaders/Sky/EqurectangluarToCube.hlsl", "cs_5_1");

		D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc{};
		computeDesc.pRootSignature = computeRootSignature.Get();
		computeDesc.CS = CD3DX12_SHADER_BYTECODE(equiToCubeShader.GetData());
		ThrowIfFailed(pDevice->GetDevice()->CreateComputePipelineState(&computeDesc, IID_PPV_ARGS(&pipelineState)));
		//ComPtr<ID3DBlob> equiToCubeShader
		auto commandList{ pDevice->GetCommandList() };
		auto barrier{ CD3DX12_RESOURCE_BARRIER::Transition(equirectangular.GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS) };
		commandList->ResourceBarrier(1, &barrier);
		ID3D12DescriptorHeap* ppHeaps[] = { pDevice->m_cbvDescriptorHeap.GetHeap() };
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		commandList->SetPipelineState(pipelineState.Get());
		commandList->SetComputeRootSignature(computeRootSignature.Get());

	}
}

void ImageBasedLighting::Prefilter()
{

	

}