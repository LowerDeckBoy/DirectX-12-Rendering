#pragma once
#include <d3d12.h>
//#include <d3dcompiler.h>
#include <wrl.h>
#include <string_view>

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
	inline ID3DBlob* GetData() const { return Blob.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3DBlob> Blob;
	bool bIsInitialized{ false };

};
