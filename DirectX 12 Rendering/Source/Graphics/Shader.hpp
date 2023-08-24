#pragma once
#include <d3dcommon.h>
#include <wrl/client.h>
#include <string_view>
//#include <d3dcompiler.h>

// Shader Model 5.x
class Shader
{
public:
	Shader() {}
	Shader(const std::string_view& Filepath, const std::string_view& Target, const std::string_view& Entrypoint = "main");
	~Shader();

	void Create(const std::string_view& Filepath, const std::string_view& Target, const std::string_view& Entrypoint = "main");
	void Reset();

	[[nodiscard]]
	inline ID3DBlob* GetData() const noexcept { return Blob.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3DBlob> Blob;
	bool bIsInitialized{ false };

};
