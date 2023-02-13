#pragma once
#include <d3d12.h>
//#include <d3dcompiler.h>
#include <wrl.h>

/*
class Shader
{
public:
	void Create(LPCWSTR ShaderPath)
	{
		std::string entryPoint = "VS";
		if (inf)
	}

private:
	Microsoft::WRL::ComPtr<ID3DBlob> Blob;
	Microsoft::WRL::ComPtr<ID3DBlob> Error;
};
*/


class VertexShader
{
public:
	void Create(LPCWSTR ShaderPath)
	{
		if (bIsInitialized)
			return;

		uint32_t compileFlags{ 0 };
#if defined (_DEBUG) || DEBUG
		compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		HRESULT hResult{ D3DCompileFromFile(ShaderPath, nullptr, nullptr, "VS", "vs_5_0", compileFlags, 0, Blob.GetAddressOf(), Error.GetAddressOf()) };

		if (FAILED(hResult) || Blob == nullptr)
		{
			OutputDebugStringA("Failed to compile Vertex Shader!\n");
			return;
		}

		if (Error != nullptr)
		{
			OutputDebugStringA(static_cast<char*>(Error->GetBufferPointer()));
			return;
		}

		bIsInitialized = true;
	}

	[[nodiscard]]
	inline ID3DBlob* GetData() const { return Blob.Get(); }

	void Release() { }



private:
	Microsoft::WRL::ComPtr<ID3DBlob> Blob;
	Microsoft::WRL::ComPtr<ID3DBlob> Error;

	bool bIsInitialized{ false };
};

class PixelShader
{
public:
	
	void Create(LPCWSTR ShaderPath)
	{

		if (bIsInitialized)
			return;

		uint32_t compileFlags{ 0 };
#if defined (_DEBUG) || DEBUG
		compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		HRESULT hResult{ D3DCompileFromFile(ShaderPath, nullptr, nullptr, "PS", "ps_5_0", compileFlags, 0, Blob.GetAddressOf(), Error.GetAddressOf()) };

		if (FAILED(hResult) || Blob == nullptr)
		{
			OutputDebugStringA("Failed to compile Vertex Shader!\n");
			return;
		}

		if (Error != nullptr)
		{
			OutputDebugStringA(static_cast<char*>(Error->GetBufferPointer()));
			return;
		}

		bIsInitialized = true;
	}

	[[nodiscard]]
	inline ID3DBlob* GetData() const { return Blob.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3DBlob> Blob;
	Microsoft::WRL::ComPtr<ID3DBlob> Error;
	bool bIsInitialized{ false };
};