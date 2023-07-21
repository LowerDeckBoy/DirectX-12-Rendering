#include "ComputePipelineState.hpp"

ComputePipelineState::ComputePipelineState()
{
}

ComputePipelineState::~ComputePipelineState()
{
    Reset();
}

void ComputePipelineState::CreateState(ID3D12Device* pDevice, ID3D12RootSignature* pRootSignature)
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
    desc.pRootSignature = pRootSignature;
    desc.CS = CD3DX12_SHADER_BYTECODE(m_ComputeShader.GetData());
    desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    ThrowIfFailed(pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddressOf())));
    m_PipelineState->SetName(L"Compute Pipeline State");
}

void ComputePipelineState::CreateRootSignature(ID3D12Device* pDevice, std::span<CD3DX12_DESCRIPTOR_RANGE1> Ranges, std::span<CD3DX12_ROOT_PARAMETER1> Parameters)
{
    const auto staticSampler{ PSOUtils::CreateStaticSampler(0) };
    const auto rootFlags    { PSOUtils::SetRootFlags() };

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootDesc{};
    rootDesc.Init_1_1(static_cast<uint32_t>(Parameters.size()), Parameters.data(), 1, &staticSampler, rootFlags);

    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;

    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, signature.GetAddressOf(), error.GetAddressOf()));
    ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.ReleaseAndGetAddressOf())));
    m_RootSignature.Get()->SetName(L"Compute Root Signature");
}

void ComputePipelineState::AddShader(const std::string_view& Filepath)
{
	Shader computeShader(Filepath.data(), "cs_5_1");
	m_ComputeShader = computeShader;
}

void ComputePipelineState::AddShader(Shader& ComputeShader)
{
	m_ComputeShader = ComputeShader;
}

void ComputePipelineState::Reset()
{
    SAFE_RELEASE(m_RootSignature);
    SAFE_RELEASE(m_PipelineState);
}

void ComputePipelineState::Dispatch(ID3D12GraphicsCommandList* pCommandList)
{

    //pCommandList->SetComputeRootSignature(m_RootSignature.Get());
    //pCommandList->SetPipelineState(m_PipelineState.Get());

    //pCommandList->Dispatch(32, 32, 1);
    pCommandList->Dispatch(1024 / 32, 1024 / 32, 1);
}
