#include "Shader.hpp"
#include <d3dcompiler.h>
#include "../Utilities/Utilities.hpp"
#include "../Utilities/Logger.hpp"


Shader::Shader(const std::string_view& Filepath, const std::string_view& Target, const std::string_view& Entrypoint)
{
	Create(Filepath, Target, Entrypoint);
}

Shader::~Shader()
{
	SAFE_RELEASE(Blob);
}

void Shader::Create(const std::string_view& Filepath, const std::string_view& Target, const std::string_view& Entrypoint)
{
	if (bIsInitialized)
		return;

	uint32_t compileFlags{ 0 };
#if defined (_DEBUG) || DEBUG
	compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ID3DBlob* Error{ nullptr };
	std::wstring path{ std::wstring(Filepath.begin(), Filepath.end()) };

	HRESULT hResult{ D3DCompileFromFile(path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, Entrypoint.data(), Target.data(), compileFlags, 0, Blob.GetAddressOf(), &Error) };

	if (Error != nullptr)
	{
		Logger::Log(static_cast<char*>(Error->GetBufferPointer()), LogType::eWarning);
	}

	if (FAILED(hResult) || Blob == nullptr)
	{
		Logger::Log("Failed to compile shader!\n", LogType::eError);
		throw std::exception();
	}

	bIsInitialized = true;
}

void Shader::Reset()
{
	SAFE_RELEASE(Blob);
	bIsInitialized = false;
}
