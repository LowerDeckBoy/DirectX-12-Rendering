#pragma once
#include <dxcapi.h>
#include <wrl/client.h>
#include <string_view>

enum class ShaderType : uint8_t
{
	eVertex = 0,
	ePixel,
	eCompute,
	eLibrary,
	eGeometry,
	eHull,
	eDomain
};

// Single instance for creating and compiling shader model 6.x
// DXIL Libraries require to create Compiler and Library
// so Manager role is to create them once
// instead of per shader basis.
// Meant for std::shared_ptr usage
class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();

	void Initialize();
	void Release();

	IDxcBlob* CreateDXIL(const std::string_view& Filepath, ShaderType eType, LPCWSTR EntryPoint = L"main");

private:
	Microsoft::WRL::ComPtr<IDxcCompiler>		m_Compiler;
	Microsoft::WRL::ComPtr<IDxcLibrary>			m_Library;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler>	m_IncludeHandler;

	constexpr LPCWSTR EnumToType(ShaderType TypeOf);

};
