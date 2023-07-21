#include "ImageBasedLighting.hpp"
#include "../Core/DeviceContext.hpp"
#include <vector>

void ImageBasedLighting::Test(DeviceContext* pDevice)
{

	//ComputePipelineState pipeline;
	m_Pipeline.AddShader("Assets/Shaders/Compute/CS_Test.hlsl");

	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges(2);
	ranges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
	ranges.at(1).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	std::vector<CD3DX12_ROOT_PARAMETER1> params(2);
	params.at(0).InitAsDescriptorTable(1, &ranges.at(0));
	params.at(1).InitAsDescriptorTable(1, &ranges.at(1));

	m_Pipeline.CreateRootSignature(pDevice->GetDevice(), ranges, params);
	m_Pipeline.CreateState(pDevice->GetDevice(), m_Pipeline.GetRootSignature());
	//m_Pipeline.

	ID3D12Resource* srvTexture{ nullptr };
	ID3D12Resource* uavTexture{ nullptr };
	//Texture unfiltered;

	D3D12_RESOURCE_DESC desc{};
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	desc.DepthOrArraySize = 1;
	desc.Width = 1024;
	desc.Height = 1024;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.MipLevels = 1;
	desc.SampleDesc = { 1, 0 };

	auto heapProps{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
	pDevice->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&srvTexture));

	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	pDevice->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&uavTexture));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

	pDevice->GetMainHeap()->Allocate(m_UAVDesc);
	pDevice->GetMainHeap()->Allocate(m_SRVDesc);

	pDevice->GetDevice()->CreateShaderResourceView(srvTexture, &srvDesc, m_SRVDesc.GetCPU());
	pDevice->GetDevice()->CreateUnorderedAccessView(uavTexture, nullptr, &uavDesc, m_UAVDesc.GetCPU());
	auto commandList{ pDevice->GetCommandList() };
	
	commandList->SetDescriptorHeaps(1, pDevice->GetMainHeap()->GetHeapAddressOf());
	commandList->SetComputeRootSignature(m_Pipeline.GetRootSignature());
	commandList->SetPipelineState(m_Pipeline.GetPipelineState());
	commandList->SetComputeRootDescriptorTable(0, m_UAVDesc.GetGPU());
	commandList->SetComputeRootDescriptorTable(1, m_SRVDesc.GetGPU());

	m_Pipeline.Dispatch(commandList);

	auto barrier{ CD3DX12_RESOURCE_BARRIER::Transition(uavTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,  D3D12_RESOURCE_STATE_COPY_SOURCE) };
	commandList->ResourceBarrier(1, &barrier);
	auto barrier2{ CD3DX12_RESOURCE_BARRIER::Transition(srvTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST) };
	commandList->ResourceBarrier(1, &barrier2);
	commandList->CopyResource(srvTexture, uavTexture);
	//D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	auto barrier3{ CD3DX12_RESOURCE_BARRIER::Transition(srvTexture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) };
	commandList->ResourceBarrier(1, &barrier3);
	//pDevice->GetDevice()->CreateUnorderedAccessView(srvTexture, nullptr, &uavDesc, m_UAVDesc.GetCPU());
	m_Texture = srvTexture;


	//m_Pipeline.Reset();

	//pDevice->FlushGPU();
	//pDevice->WaitForGPU();
}

void ImageBasedLighting::Test2(DeviceContext* pDevice, const std::string_view& Filepath)
{
	//m_Pipeline.AddShader("Assets/Shaders/Compute/CS_Test.hlsl");
	m_Pipeline.AddShader("Assets/Shaders/Compute/SampleSkybox.hlsl");

	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges(3);
	ranges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE);
	ranges.at(1).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	ranges.at(2).Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
	std::vector<CD3DX12_ROOT_PARAMETER1> params(2);
	params.at(0).InitAsDescriptorTable(1, &ranges.at(0));
	params.at(1).InitAsDescriptorTable(1, &ranges.at(1));

	m_Pipeline.CreateRootSignature(pDevice->GetDevice(), ranges, params);
	m_Pipeline.CreateState(pDevice->GetDevice(), m_Pipeline.GetRootSignature());

	ID3D12Resource* hdrTexture{ nullptr };
	ID3D12Resource* uavTexture{ nullptr };

	//TextureUtils::CreateFromWIC(pDevice->GetDevice(), srvTexture, Filepath, {});
	//TextureUtils::CreateResource(pDevice->GetDevice(), uavTexture, {}, { 1024, 1024 });
	hdrTexture = TextureUtils::CreateFromHDR(pDevice->GetDevice(), pDevice->GetCommandList(), Filepath, {});
	//uavTexture = TextureUtils::CreateResource(pDevice->GetDevice(), { D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }, { 6144, 3072 });
	uavTexture = TextureUtils::CreateResource(pDevice->GetDevice(), { D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }, { 1600, 800, DXGI_FORMAT_R32G32B32A32_FLOAT });

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	//srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	//srvDesc.TextureCube.MipLevels = 1;
	//srvDesc.TextureCube.MostDetailedMip = 0;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

	Descriptor temp;
	pDevice->GetMainHeap()->Allocate(m_UAVDesc);
	pDevice->GetMainHeap()->Allocate(temp);

	pDevice->GetDevice()->CreateShaderResourceView(hdrTexture, &srvDesc, temp.GetCPU());
	pDevice->GetDevice()->CreateUnorderedAccessView(uavTexture, nullptr, &uavDesc, m_UAVDesc.GetCPU());
	auto commandList{ pDevice->GetCommandList() };

	//=================== Dispatch ===========================
	commandList->SetDescriptorHeaps(1, pDevice->GetMainHeap()->GetHeapAddressOf());
	commandList->SetComputeRootSignature(m_Pipeline.GetRootSignature());
	commandList->SetPipelineState(m_Pipeline.GetPipelineState());
	commandList->SetComputeRootDescriptorTable(0, m_UAVDesc.GetGPU());
	commandList->SetComputeRootDescriptorTable(1, temp.GetGPU());

	m_Pipeline.Dispatch(commandList);

	auto barrier{ CD3DX12_RESOURCE_BARRIER::Transition(uavTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,  D3D12_RESOURCE_STATE_COPY_SOURCE) };
	commandList->ResourceBarrier(1, &barrier);
	auto barrier2{ CD3DX12_RESOURCE_BARRIER::Transition(hdrTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST) };
	commandList->ResourceBarrier(1, &barrier2);
	//commandList->CopyResource(hdrTexture, uavTexture);

	// https://github.com/Nadrin/PBR/blob/master/src/d3d12.cpp
	// Convert from Texture2D to TextureCube here?
	for (uint32_t arraySlice = 0; arraySlice < 6; ++arraySlice) {
		const UINT subresourceIndex = D3D12CalcSubresource(0, arraySlice, 0, 1, 6);

		auto copyDst{ CD3DX12_TEXTURE_COPY_LOCATION{ hdrTexture, subresourceIndex } };
		auto copySrc{ CD3DX12_TEXTURE_COPY_LOCATION{ uavTexture, subresourceIndex } };
		
		commandList->CopyTextureRegion(&copyDst, 0, 0, 0, &copySrc, nullptr);
	}

	//
	auto barrier3{ CD3DX12_RESOURCE_BARRIER::Transition(hdrTexture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) };
	commandList->ResourceBarrier(1, &barrier3);

	// TEST
	D3D12_SHADER_RESOURCE_VIEW_DESC srvCubeDesc{};
	//srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvCubeDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvCubeDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvCubeDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvCubeDesc.TextureCube.MipLevels = 1;
	srvCubeDesc.TextureCube.MostDetailedMip = 0;

	pDevice->GetMainHeap()->Allocate(m_SRVDesc);
	pDevice->GetDevice()->CreateShaderResourceView(hdrTexture, &srvDesc, m_SRVDesc.GetCPU());

	m_Texture = hdrTexture;

}
