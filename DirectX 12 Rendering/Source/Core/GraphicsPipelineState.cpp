#include "../Graphics/Shader.hpp"
#include "../Graphics/ShaderManager.hpp"
#include "GraphicsPipelineState.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Utilities/Logger.hpp"
//#include "../Core/DeviceContext.hpp"


PSOBuilder::PSOBuilder()
{
}

PSOBuilder::~PSOBuilder()
{
	if (m_ShaderManager)
		m_ShaderManager = nullptr;

	Reset();
	Logger::Log("PSOBuilder released.");
}

void PSOBuilder::AddShaderManger(ShaderManager* pShaderManager)
{
	if (!m_ShaderManager)
	{
		m_ShaderManager = pShaderManager;
		assert(m_ShaderManager && "Failed to assign ShaderManager!");
	}
	//assert(m_ShaderManager = pShaderManager);
}

void PSOBuilder::Create(ID3D12Device* pDevice, ID3D12RootSignature* pRootSignature, ID3D12PipelineState** ppTarget, LPCWSTR DebugName)
{
	// reset object if ppTarget isn't empty 
	// meant for reusing Builder without clearing it whole each time
	if ((*ppTarget) != nullptr)
	{
		(*ppTarget)->Release();
		(*ppTarget) = nullptr;
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
	desc.pRootSignature = pRootSignature;
	desc.InputLayout = { m_InputLayout.data(), static_cast<uint32_t>(m_InputLayout.size()) };

	desc.VS = CD3DX12_SHADER_BYTECODE(m_VertexShader->GetBufferPointer(), m_VertexShader->GetBufferSize());
	desc.PS = CD3DX12_SHADER_BYTECODE(m_PixelShader->GetBufferPointer(), m_PixelShader->GetBufferSize());

	if (m_DomainShader)
		desc.DS = CD3DX12_SHADER_BYTECODE(m_DomainShader->GetBufferPointer(), m_DomainShader->GetBufferSize());
	if (m_HullShader)
		desc.HS = CD3DX12_SHADER_BYTECODE(m_HullShader->GetBufferPointer(), m_HullShader->GetBufferSize());
	if (m_GeometryShader)
		desc.GS = CD3DX12_SHADER_BYTECODE(m_GeometryShader->GetBufferPointer(), m_GeometryShader->GetBufferSize());

	desc.RasterizerState = m_RasterizerState;
	desc.RasterizerState.FillMode = m_FillMode;
	desc.RasterizerState.CullMode = m_CullMode;
	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	desc.DepthStencilState = m_DepthDesc;
	desc.SampleMask = UINT_MAX;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	//desc.PrimitiveTopologyType = (m_DomainShader ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.SampleDesc = { 1, 0 };

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootDesc{};
	rootDesc.Init_1_1(static_cast<uint32_t>(m_Parameters.size()), m_Parameters.data(), 1, &m_StaticSampler, m_RootFlags);

	ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(ppTarget)), "Failed to create PipelineState!");
	//ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&desc, __uuidof(ID3D12PipelineState), (void**)ppTarget.GetAddressOf()), "Failed to create PipelineState!");

	if (DebugName)
		(*ppTarget)->SetName(DebugName);

}

void PSOBuilder::CreateRootSignature(ID3D12Device* pDevice, ID3D12RootSignature** ppTarget, LPCWSTR DebugName)
{
	if ((*ppTarget) != nullptr)
	{
		(*ppTarget)->Release();
		(*ppTarget) = nullptr;
	}

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootDesc{};
	rootDesc.Init_1_1(static_cast<uint32_t>(m_Parameters.size()), m_Parameters.data(), 1, &m_StaticSampler, m_RootFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;

	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, signature.GetAddressOf(), error.GetAddressOf()));

	ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(ppTarget)), "Failed to create ID3D12RootSignature!");
	if (DebugName)
		(*ppTarget)->SetName(DebugName);

}

void PSOBuilder::AddRanges(const std::vector<CD3DX12_DESCRIPTOR_RANGE1>& Ranges)
{
	m_Ranges.clear();
	m_Ranges.insert(m_Ranges.begin(), Ranges.begin(), Ranges.end());
}

void PSOBuilder::AddParameters(const std::vector<CD3DX12_ROOT_PARAMETER1>& Parameters)
{
	m_Parameters.clear();
	m_Parameters.insert(m_Parameters.begin(), Parameters.begin(), Parameters.end());
}

void PSOBuilder::AddInputLayout(const std::span<D3D12_INPUT_ELEMENT_DESC>& Layout)
{
	m_InputLayout.clear();
	m_InputLayout.insert(m_InputLayout.begin(), Layout.begin(), Layout.end());
}

void PSOBuilder::AddSampler(uint32_t ShaderRegister, uint32_t RegisterSpace, D3D12_FILTER Filter, D3D12_TEXTURE_ADDRESS_MODE AddressMode, D3D12_COMPARISON_FUNC ComparsionFunc)
{
	//D3D12_STATIC_SAMPLER_DESC desc{};
	m_StaticSampler.AddressU = AddressMode;
	m_StaticSampler.AddressV = AddressMode;
	m_StaticSampler.AddressW = AddressMode;
	m_StaticSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	m_StaticSampler.ComparisonFunc = ComparsionFunc;
	m_StaticSampler.Filter = Filter;
	m_StaticSampler.MaxAnisotropy = 0;
	m_StaticSampler.MinLOD = 0.0f;
	m_StaticSampler.MaxLOD = static_cast<float>(UINT32_MAX);
	m_StaticSampler.ShaderRegister = ShaderRegister;
	m_StaticSampler.RegisterSpace = RegisterSpace;
	m_StaticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

}

void PSOBuilder::AddShaders(const std::string_view& VertexPath, const std::string_view& PixelPath)
{
	//m_VertexShader.Reset();
	//m_PixelShader.Reset();

	//m_VertexShader.Create(VertexPath.data(), "vs_5_1");
	//m_PixelShader.Create(PixelPath.data(), "ps_5_1");

	m_VertexShader = nullptr;
	m_PixelShader = nullptr;
	
	m_VertexShader = m_ShaderManager->CreateDXIL(VertexPath, L"vs_6_0");
	m_PixelShader = m_ShaderManager->CreateDXIL(PixelPath, L"ps_6_0");

}

void PSOBuilder::AddGeometryShader(const std::string_view& GeometryPath)
{
	m_GeometryShader = m_ShaderManager->CreateDXIL(GeometryPath, L"gs_6_0");
}

void PSOBuilder::AddDomainShader(const std::string_view& DomainPath)
{
	m_DomainShader = m_ShaderManager->CreateDXIL(DomainPath, L"ds_6_0");
}

void PSOBuilder::AddHullShader(const std::string_view& HullPath)
{
	m_HullShader = m_ShaderManager->CreateDXIL(HullPath, L"ds_6_0");
}

void PSOBuilder::SetRasterizerState(CD3DX12_RASTERIZER_DESC RasterizerState)
{
	m_RasterizerState = RasterizerState;
}

void PSOBuilder::SetFillMode(D3D12_FILL_MODE FillMode)
{
	m_FillMode = FillMode;
}

void PSOBuilder::SetCullMode(D3D12_CULL_MODE CullMode)
{
	m_CullMode = CullMode;
}

void PSOBuilder::AddRootFlags(D3D12_ROOT_SIGNATURE_FLAGS Flags)
{
	m_RootFlags = Flags;
}

void PSOBuilder::AddDepthStencil(CD3DX12_DEPTH_STENCIL_DESC DepthDesc)
{
	m_DepthDesc = DepthDesc;
}

void PSOBuilder::Reset()
{
	m_Ranges.clear();
	m_Parameters.clear();
	m_InputLayout = {};

	m_StaticSampler = {};
	m_RootFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	
	m_RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	m_FillMode = D3D12_FILL_MODE_SOLID;
	m_CullMode = D3D12_CULL_MODE_NONE;

	m_DepthDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	
}

// ======================= Utils =======================
D3D12_GRAPHICS_PIPELINE_STATE_DESC PSOUtils::CreateState(ID3D12RootSignature* pRootSignature, Shader* pVertexShader, Shader* pPixelShader, std::span<D3D12_INPUT_ELEMENT_DESC> InputLayout)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
	desc.pRootSignature = pRootSignature;
	desc.InputLayout = { InputLayout.data(), static_cast<uint32_t>(InputLayout.size()) };
	desc.VS = CD3DX12_SHADER_BYTECODE(pVertexShader->GetData());
	desc.PS = CD3DX12_SHADER_BYTECODE(pPixelShader->GetData());
	desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	//desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
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

D3D12_STATIC_SAMPLER_DESC PSOUtils::CreateStaticSampler(uint32_t ShaderRegister, uint32_t RegisterSpace, D3D12_FILTER Filter, D3D12_TEXTURE_ADDRESS_MODE AddressMode, D3D12_COMPARISON_FUNC ComparsionFunc)
{
	D3D12_STATIC_SAMPLER_DESC desc{};
	desc.AddressU = AddressMode;
	desc.AddressV = AddressMode;
	desc.AddressW = AddressMode;
	desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	desc.ComparisonFunc = ComparsionFunc;
	desc.Filter = Filter;
	desc.MaxAnisotropy = 0;
	desc.MinLOD = 0.0f;
	desc.MaxLOD = static_cast<float>(UINT32_MAX);
	desc.ShaderRegister = ShaderRegister;
	desc.RegisterSpace = RegisterSpace;
	desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	return desc;
}

D3D12_SAMPLER_DESC PSOUtils::CreateSampler(D3D12_FILTER Filter, D3D12_TEXTURE_ADDRESS_MODE AddressMode, D3D12_COMPARISON_FUNC ComparsionFunc)
{
	D3D12_SAMPLER_DESC desc{};
	desc.AddressU = AddressMode;
	desc.AddressV = AddressMode;
	desc.AddressW = AddressMode;
	desc.ComparisonFunc = ComparsionFunc;
	desc.Filter = Filter;
	desc.MaxAnisotropy = 0;
	desc.MinLOD = 0.0f;
	desc.MaxLOD = static_cast<float>(UINT32_MAX);

	return desc;
}

D3D12_ROOT_SIGNATURE_FLAGS PSOUtils::SetRootFlags(D3D12_ROOT_SIGNATURE_FLAGS Flags)
{
	D3D12_ROOT_SIGNATURE_FLAGS flags = Flags;
	flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
	flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	return flags;
}

std::array<D3D12_INPUT_ELEMENT_DESC, 5> PSOUtils::CreateInputLayout()
{
	std::array<D3D12_INPUT_ELEMENT_DESC, 5> layout{
		D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			D3D12_INPUT_ELEMENT_DESC{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			D3D12_INPUT_ELEMENT_DESC{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			D3D12_INPUT_ELEMENT_DESC{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	return layout;
}

std::array<D3D12_INPUT_ELEMENT_DESC, 2> PSOUtils::CreateSkyboxInputLayout()
{
	std::array<D3D12_INPUT_ELEMENT_DESC, 2> layout{
		D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	return layout;
}

std::array<D3D12_INPUT_ELEMENT_DESC, 2> PSOUtils::CreateScreenQuadLayout()
{
	std::array<D3D12_INPUT_ELEMENT_DESC, 2> layout{
		D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	return layout;
}
