#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl.h>

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

		HRESULT hResult{ D3DCompileFromFile(ShaderPath, nullptr, nullptr, "VS", "vs_5_1", compileFlags, 0, Blob.GetAddressOf(), Error.GetAddressOf()) };

		if (FAILED(hResult) || Blob == nullptr)
		{
			::OutputDebugStringA(static_cast<char*>(Blob->GetBufferPointer()));
			::OutputDebugStringA("Failed to compile Vertex Shader!\n");
			return;
		}

		if (Error != nullptr)
		{
			::OutputDebugStringA(static_cast<char*>(Error->GetBufferPointer()));
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

		HRESULT hResult{ D3DCompileFromFile(ShaderPath, nullptr, nullptr, "PS", "ps_5_1", compileFlags, 0, Blob.GetAddressOf(), Error.GetAddressOf()) };

		if (FAILED(hResult) || Blob == nullptr)
		{
			::OutputDebugStringA("Failed to compile Pixel Shader!\n");
			return;
		}

		if (Error != nullptr)
		{
			::OutputDebugStringA(static_cast<char*>(Error->GetBufferPointer()));
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

class Shader
{
public:
	void Create(const std::string_view& Filepath, const std::string_view& Entrypoint, const std::string_view& Target, ID3DInclude* pInclude = nullptr)
	{
		if (bIsInitialized)
			return;

		uint32_t compileFlags{ 0 };
#if defined (_DEBUG) || DEBUG
		compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		ID3DBlob* Error{ nullptr };
		std::wstring path{ std::wstring(Filepath.begin(), Filepath.end()) };
		HRESULT hResult{ D3DCompileFromFile(path.c_str(), nullptr, pInclude, Entrypoint.data(), Target.data(), compileFlags, 0, Blob.GetAddressOf(), &Error)};

		if (Error != nullptr)
		{
			::OutputDebugStringA(static_cast<char*>(Error->GetBufferPointer()));
			return;
		}

		if (FAILED(hResult) || Blob == nullptr)
		{
			::OutputDebugStringA("Failed to compile shader!\n");
			return;
		}

		bIsInitialized = true;
	}

	[[nodiscard]]
	inline ID3DBlob* GetData() const { return Blob.Get(); }


private:
	Microsoft::WRL::ComPtr<ID3DBlob> Blob;
	bool bIsInitialized{ false };
};
