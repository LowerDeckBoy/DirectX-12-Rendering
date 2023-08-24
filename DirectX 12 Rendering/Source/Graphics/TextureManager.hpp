#pragma once
#include <unordered_map>

class DeviceContext;
class Texture;

// https://github.com/mateeeeeee/Adria-DX12/blob/master/Adria/Rendering/TextureManager.h

// TODO:
// Single object for loading and storing all textures
// responsible for creating MipChains
class TextureManager
{
public:
	TextureManager(DeviceContext* pDeviceCtx);
	~TextureManager();

	void Initialize();

	Texture2D CreateTexture2D();
	Texture3D CreateTexture3D();

	void GenerateMipmaps(Texture2D& Texture);
	void GenerateMipmaps(Texture3D& Cubemap);

private:
	DeviceContext* m_DeviceCtx{ nullptr };

	std::unordered_map<uint32_t, Texture2D> m_Textures;
	std::unordered_map<uint32_t, Texture3D> m_Cubemaps;

};
