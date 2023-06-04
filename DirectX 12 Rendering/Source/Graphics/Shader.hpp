#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include "../Utils/Utilities.hpp"


class Shader
{
public:
	Shader() {}
	Shader(const std::string_view& Filepath, const std::string_view& Target, const std::string_view& Entrypoint = "main", ID3DInclude* pInclude = nullptr)
	{
		Create(Filepath, Target, Entrypoint, pInclude);
	}

	void Create(const std::string_view& Filepath, const std::string_view& Target, const std::string_view& Entrypoint = "main", ID3DInclude* pInclude = nullptr)
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

	void Reset()
	{
		SAFE_RELEASE(Blob);
		bIsInitialized = false;
	}

	[[nodiscard]]
	inline ID3DBlob* GetData() const { return Blob.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3DBlob> Blob;
	bool bIsInitialized{ false };
};
