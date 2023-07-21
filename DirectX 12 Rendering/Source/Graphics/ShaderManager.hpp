#pragma once
#include <dxcapi.h>
#include <wrl.h>
#include <string_view>

// Single instance for creating and compiling shader model 6.x
// DXIL Libraries require to create Compiler and Library
// so Manager role is to create them once
// instead of per shader basis
class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();

	void Initialize();
	void Release();

	IDxcBlob* CreateDXIL(const std::string_view& Filepath, LPCWSTR Target, LPCWSTR EntryPoint = L"main");

private:
	Microsoft::WRL::ComPtr<IDxcCompiler>		m_Compiler;
	Microsoft::WRL::ComPtr<IDxcLibrary>			m_Library;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler>	m_IncludeHandler;

};