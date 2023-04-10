#include "../Core/Device.hpp"
#include "../Graphics/Shader.hpp"
#include "GraphicsPipelineState.hpp"

D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsPipelineState::CreateState(ID3D12RootSignature* pRootSignature, Shader* pVertexShader, Shader* pPixelShader, std::span<D3D12_INPUT_ELEMENT_DESC> InputLayout)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
    desc.pRootSignature = pRootSignature;
    desc.InputLayout = { InputLayout.data(), static_cast<uint32_t>(InputLayout.size()) };
    desc.VS = CD3DX12_SHADER_BYTECODE(pVertexShader->GetData());
    desc.PS = CD3DX12_SHADER_BYTECODE(pPixelShader->GetData());
    desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    desc.SampleMask = UINT_MAX;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.SampleDesc = { 1, 0 };

    return desc;
}

D3D12_STATIC_SAMPLER_DESC GraphicsPipelineState::CreateStaticSampler(uint32_t ShaderRegister, uint32_t RegisterSpace, D3D12_FILTER Filter, D3D12_TEXTURE_ADDRESS_MODE AddressMode, D3D12_COMPARISON_FUNC ComparsionFunc)
{
    D3D12_STATIC_SAMPLER_DESC desc{};
    desc.AddressU = AddressMode;
    desc.AddressV = AddressMode;
    desc.AddressW = AddressMode;
    desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    desc.ComparisonFunc = ComparsionFunc;
    desc.Filter = Filter;
    desc.MaxAnisotropy = 0;
    desc.MinLOD = 0;
    desc.MaxLOD = UINT32_MAX;
    desc.ShaderRegister = ShaderRegister;
    desc.RegisterSpace = RegisterSpace;
    desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    return desc;
}

D3D12_SAMPLER_DESC GraphicsPipelineState::CreateSampler(D3D12_FILTER Filter, D3D12_TEXTURE_ADDRESS_MODE AddressMode, D3D12_COMPARISON_FUNC ComparsionFunc)
{
    D3D12_SAMPLER_DESC desc{};
    desc.AddressU = AddressMode;
    desc.AddressV = AddressMode;
    desc.AddressW = AddressMode;
    desc.ComparisonFunc = ComparsionFunc;
    desc.Filter = Filter;
    desc.MaxAnisotropy = 0;
    desc.MinLOD = 0;
    desc.MaxLOD = UINT32_MAX;

    return desc;
}

D3D12_ROOT_SIGNATURE_FLAGS GraphicsPipelineState::SetRootFlags(D3D12_ROOT_SIGNATURE_FLAGS Flags)
{
    D3D12_ROOT_SIGNATURE_FLAGS flags = Flags;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

    return flags;
}

std::array<D3D12_INPUT_ELEMENT_DESC, 5> GraphicsPipelineState::CreateInputLayout()
{
    std::array<D3D12_INPUT_ELEMENT_DESC, 5> layout{
        D3D12_INPUT_ELEMENT_DESC{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        D3D12_INPUT_ELEMENT_DESC{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        D3D12_INPUT_ELEMENT_DESC{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        D3D12_INPUT_ELEMENT_DESC{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
    return layout;
}

std::array<D3D12_INPUT_ELEMENT_DESC, 2> GraphicsPipelineState::CreateSkyboxInputLayout()
{
    std::array<D3D12_INPUT_ELEMENT_DESC, 2> layout{
        D3D12_INPUT_ELEMENT_DESC{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    return layout;
}
