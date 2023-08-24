#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <span>
//#include <dxcapi.h>
//#include <wrl.h>

class Shader;
class ShaderManager;
struct IDxcBlob;

using Microsoft::WRL::ComPtr;

class PSOBuilder
{
public:
	PSOBuilder();
	~PSOBuilder();

	void AddShaderManger(ShaderManager* pShaderManager);

	void Create(ID3D12Device* pDevice, ID3D12RootSignature* pRootSignature, ID3D12PipelineState** pTarget, LPCWSTR DebugName = L"");
	void CreateRootSignature(ID3D12Device* pDevice, ID3D12RootSignature** ppTarget, LPCWSTR DebugName = L"");
	void AddRanges(const std::vector<CD3DX12_DESCRIPTOR_RANGE1>& Ranges);
	void AddParameters(const std::vector<CD3DX12_ROOT_PARAMETER1>& Parameters);
	void AddInputLayout(const std::span<D3D12_INPUT_ELEMENT_DESC>& Layout);
	void AddSampler(uint32_t ShaderRegister,
		uint32_t RegisterSpace = 0,
		D3D12_FILTER Filter = D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE AddressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_COMPARISON_FUNC ComparsionFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL);
	void AddShaders(const std::string_view& VertexPath, const std::string_view& PixelPath);
	void AddGeometryShader(const std::string_view& GeometryPath);
	void AddDomainShader(const std::string_view& DomainPath);
	void AddHullShader(const std::string_view& HullPath);
	void SetRasterizerState(CD3DX12_RASTERIZER_DESC RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT));
	void SetFillMode(D3D12_FILL_MODE FillMode);
	void SetCullMode(D3D12_CULL_MODE CullMode);
	void AddRootFlags(D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	void AddDepthStencil(CD3DX12_DEPTH_STENCIL_DESC DepthDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));
	void Reset();
	

private:
	std::vector<CD3DX12_DESCRIPTOR_RANGE1>	m_Ranges;
	std::vector<CD3DX12_ROOT_PARAMETER1>	m_Parameters;
	std::vector<D3D12_INPUT_ELEMENT_DESC>	m_InputLayout;

	IDxcBlob* m_VertexShader  { nullptr };
	IDxcBlob* m_PixelShader   { nullptr };
	IDxcBlob* m_DomainShader  { nullptr };
	IDxcBlob* m_HullShader	  { nullptr };
	IDxcBlob* m_GeometryShader{ nullptr };

	D3D12_STATIC_SAMPLER_DESC m_StaticSampler{};
	D3D12_ROOT_SIGNATURE_FLAGS m_RootFlags{ D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT };
	D3D12_CULL_MODE m_CullMode{ D3D12_CULL_MODE_BACK };
	D3D12_FILL_MODE m_FillMode{ D3D12_FILL_MODE_SOLID };

	CD3DX12_DEPTH_STENCIL_DESC m_DepthDesc{ CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT) };
	CD3DX12_RASTERIZER_DESC m_RasterizerState{ CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT) };

private:
	ShaderManager* m_ShaderManager{ nullptr };

};

class PSOUtils
{
public:
	static D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateState(ID3D12RootSignature* pRootSignature, Shader* pVertexShader, Shader* pPixelShader, std::span<D3D12_INPUT_ELEMENT_DESC> InputLayout);

	static D3D12_STATIC_SAMPLER_DESC CreateStaticSampler(uint32_t ShaderRegister,
		uint32_t RegisterSpace = 0,
		D3D12_FILTER Filter = D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE AddressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_COMPARISON_FUNC ComparsionFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL);

	static D3D12_SAMPLER_DESC CreateSampler(D3D12_FILTER Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR, 
		D3D12_TEXTURE_ADDRESS_MODE AddressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP, 
		D3D12_COMPARISON_FUNC ComparsionFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL);

	// Defaults to allowing INPUT_ASSEMBLER_INPUT_LAYOUT only
	static D3D12_ROOT_SIGNATURE_FLAGS SetRootFlags(D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// Default layout contains 3D model format:
	// Position, TexCoord, Normal,Tangent, Bitangent
	static std::array<D3D12_INPUT_ELEMENT_DESC, 5> CreateInputLayout();

	//static std::array<D3D12_INPUT_ELEMENT_DESC, 5> CreateGBufferLayout();

	static std::array<D3D12_INPUT_ELEMENT_DESC, 2> CreateSkyboxInputLayout();

	static std::array<D3D12_INPUT_ELEMENT_DESC, 2> CreateScreenQuadLayout();

};
