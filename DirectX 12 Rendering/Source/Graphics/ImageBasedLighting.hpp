#pragma once
#include "../Core/Device.hpp"
#include "Texture.hpp"
#include "Shader.hpp"

using Microsoft::WRL::ComPtr;
class ImageBasedLighting
{
public:

	void Create(Device* pDevice);
	void Prefilter();

private:
	Texture m_EnvironmentMap;
	Texture m_IrradianceMap;

	// Compute root signature

};

