#pragma once
#include <d3d12.h>
#include <wrl.h>

#include <string>
#include <DirectXTex.h>

class Device;

class Texture
{
public:
	Texture();
	Texture(Device* pDevice, const std::string& TexturePath);
	~Texture();

	void Initialize(Device* pDevice, const std::string& TexturePath);

	void Release();

private:
	Device* m_Device;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_Texture;
	// Temporal solution
	// Making it local causes problems
	// with uploading to GPU
	// before lifetime ends
	Microsoft::WRL::ComPtr<ID3D12Resource> m_TextureUploadHeap;

};