#include <d3d12.h>
#include "ShaderManager.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Utilities/Logger.hpp"
#include <fstream>
#include <sstream>
#include <vector>


ShaderManager::ShaderManager()
{
	Initialize();
}

ShaderManager::~ShaderManager()
{
	Release();
}

void ShaderManager::Initialize()
{
	if (m_Compiler.Get())
	{
		Logger::Log("ShaderManager is already initialized.", LogType::eWarning);
		return;
	}

	ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(m_Compiler.ReleaseAndGetAddressOf())));
	ThrowIfFailed(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(m_Library.ReleaseAndGetAddressOf())));
	ThrowIfFailed(m_Library.Get()->CreateIncludeHandler(m_IncludeHandler.ReleaseAndGetAddressOf()));
}

IDxcBlob* ShaderManager::CreateDXIL(const std::string_view& Filepath, LPCWSTR Target, LPCWSTR EntryPoint)
{
	std::ifstream shaderFile(Filepath.data());
	if (!shaderFile.good())
		throw std::logic_error("Failed to read shader file!");

	std::stringstream strStream;
	strStream << shaderFile.rdbuf();
	std::string shaderStr{ strStream.str() };

	IDxcBlobEncoding* textBlob{};
	ThrowIfFailed(m_Library.Get()->CreateBlobWithEncodingFromPinned(LPBYTE(shaderStr.c_str()), static_cast<uint32_t>(shaderStr.size()), 0, &textBlob));

	IDxcOperationResult* result{};

	std::wstring wstr{ std::wstring(Filepath.begin(), Filepath.end()) };
	LPCWSTR filepath{ wstr.c_str() };
	ThrowIfFailed(m_Compiler.Get()->Compile(textBlob, filepath, EntryPoint, Target, nullptr, 0, nullptr, 0, m_IncludeHandler.Get(), &result));

	HRESULT resultCode{};
	ThrowIfFailed(result->GetStatus(&resultCode));
	if (FAILED(resultCode))
	{
		IDxcBlobEncoding* error{};
		HRESULT hResult{ result->GetErrorBuffer(&error) };
		if (FAILED(hResult))
		{
			throw std::logic_error("Failed to get shader error code!");
		}

		std::vector<char> infoLog(error->GetBufferSize() + 1);
		std::memcpy(infoLog.data(), error->GetBufferPointer(), error->GetBufferSize());
		infoLog[error->GetBufferSize()] = 0;

		std::string errorMsg{ "Shader Compiler error:\n" };
		errorMsg.append(infoLog.data());

		::OutputDebugStringA(errorMsg.c_str());
		Logger::Log(errorMsg.c_str(), LogType::eError);
		throw std::exception();
	}

	IDxcBlob* blob{ nullptr };
	ThrowIfFailed(result->GetResult(&blob));
	return blob;
}

void ShaderManager::Release()
{
	SAFE_RELEASE(m_IncludeHandler);
	SAFE_RELEASE(m_Library);
	SAFE_RELEASE(m_Compiler);

	Logger::Log("ShaderManager released.");
}
