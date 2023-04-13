#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <array>
#include "../Core/Device.hpp"
#include "../Graphics/Shader.hpp"
#include "GraphicsPipelineState.hpp"
#include "../Utils/Utils.hpp"

//class Shader;

// https://logins.github.io/graphics/2020/10/31/D3D12ComputeShaders.html
class ComputePipelineState
{
public:

    void Create(Device* pDevice, const Shader& ComputeShader)
    {
        CreateRootSignature(pDevice->GetDevice());
        CreateState(pDevice->GetDevice(), m_RootSignature.Get(), ComputeShader);
    }

	void CreateState(ID3D12Device* pDevice, ID3D12RootSignature* pRootSignature, const Shader& ComputeShader)
	{
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
        desc.pRootSignature = pRootSignature;
        desc.CS = CD3DX12_SHADER_BYTECODE(ComputeShader.GetData());
        desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

        struct PipelineStateStream
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE RootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_CS ComputeShader;
        } pipelineStateStream;

        pipelineStateStream.RootSignature = pRootSignature;
        pipelineStateStream.ComputeShader = CD3DX12_SHADER_BYTECODE(ComputeShader.GetData());

       // D3D12_PIPELINE_STATE_STREAM_DESC stateDesc{ sizeof(pipelineStateStream), &pipelineStateStream };
        ThrowIfFailed(pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddressOf())));
        //return desc;
	}

   void CreateRootSignature(ID3D12Device* pDevice)
    {
        std::array<CD3DX12_DESCRIPTOR_RANGE1, 1> ranges{};
        ranges.at(0).Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
        std::array<CD3DX12_ROOT_PARAMETER1, 1> params{};
        params.at(0).InitAsDescriptorTable(1, &ranges.at(0), D3D12_SHADER_VISIBILITY_ALL);

        const auto staticSampler{ GraphicsPipelineState::CreateStaticSampler(0) };
        const auto rootFlags    { GraphicsPipelineState::SetRootFlags() };

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootDesc{};
        rootDesc.Init_1_1(static_cast<uint32_t>(params.size()), params.data(), 1, &staticSampler, rootFlags);

        Microsoft::WRL::ComPtr<ID3DBlob> signature;
        Microsoft::WRL::ComPtr<ID3DBlob> error;

        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, signature.GetAddressOf(), error.GetAddressOf()));
        ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.ReleaseAndGetAddressOf())));
    }

   // Get Compute Root Signature
   ID3D12RootSignature* GetRootSignature() const { return m_RootSignature.Get(); }
   // Get Compute Pipeline State
   ID3D12PipelineState* GetPipelineState() const { return m_PipelineState.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;

};

