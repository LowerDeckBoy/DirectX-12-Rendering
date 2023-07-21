#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <array>
#include "../Core/DeviceContext.hpp"
#include "../Graphics/Shader.hpp"
#include "GraphicsPipelineState.hpp"
#include "../Utilities/Utilities.hpp"

// https://logins.github.io/graphics/2020/10/31/D3D12ComputeShaders.html
// https://www.3dgep.com/learning-directx-12-4/#Compute_Shaders

class ComputePipelineState
{
public:
    ComputePipelineState();
    ~ComputePipelineState();

    void Create(DeviceContext* pDevice);
    void CreateState(ID3D12Device* pDevice, ID3D12RootSignature* pRootSignature);
    void CreateRootSignature(ID3D12Device* pDevice, std::span<CD3DX12_DESCRIPTOR_RANGE1> Ranges, std::span<CD3DX12_ROOT_PARAMETER1> Parameters);
    // Create and bind shader inside
    void AddShader(const std::string_view& Filepath);
    // Add precompiled shader
    void AddShader(Shader& ComputeShader);
    void AddSampler();

    void Reset();

    void Dispatch(ID3D12GraphicsCommandList* pCommandList);

   // Get Compute Root Signature
   ID3D12RootSignature* GetRootSignature() const { return m_RootSignature.Get(); }
   // Get Compute Pipeline State
   ID3D12PipelineState* GetPipelineState() const { return m_PipelineState.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
    Shader m_ComputeShader;

};
