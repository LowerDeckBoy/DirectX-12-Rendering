#pragma once
#include <d3d12.h>
#include <d3dx12.h>

#include <span>

class Shader;
class Shader;

class GraphicsPipelineState
{
public:
	static D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateState(ID3D12RootSignature* pRootSignature, Shader* pVertexShader, Shader* pPixelShader, std::span<D3D12_INPUT_ELEMENT_DESC> InputLayout);

	static D3D12_STATIC_SAMPLER_DESC CreateStaticSampler(uint32_t ShaderRegister,
		uint32_t RegisterSpace = 0,
		D3D12_FILTER Filter = D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE AddressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_COMPARISON_FUNC ComparsionFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL);

	static D3D12_SAMPLER_DESC CreateSampler(D3D12_FILTER Filter = D3D12_FILTER_ANISOTROPIC, 
		D3D12_TEXTURE_ADDRESS_MODE AddressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP, 
		D3D12_COMPARISON_FUNC ComparsionFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL);

	// Default to allowing INPUT_ASSEMBLER_INPUT_LAYOUT only
	static D3D12_ROOT_SIGNATURE_FLAGS SetRootFlags(D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// Default layout contains 3D model data:
	// Position, TexCoord, Normal,Tangent, Bitangent
	static std::array<D3D12_INPUT_ELEMENT_DESC, 5> CreateInputLayout();

	static std::array<D3D12_INPUT_ELEMENT_DESC, 2> CreateSkyboxInputLayout();

private:

};

