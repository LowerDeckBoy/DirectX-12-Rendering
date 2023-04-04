#pragma once
#include <DirectXMath.h>
#include "../Graphics/Texture.hpp"

using namespace DirectX;
struct Material
{
	Material() = default;
	Material operator=(const Material& rhs)
	{
		BaseColor = rhs.BaseColor;
		EmissiveFactor = rhs.EmissiveFactor;
		MetallicFactor = rhs.MetallicFactor;
		RoughnessFactor = rhs.RoughnessFactor;
		AlphaCutoff = rhs.AlphaCutoff;
		BaseColorTexture = rhs.BaseColorTexture;
		NormalTexture = rhs.NormalTexture;
	}

	XMFLOAT4 BaseColor{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
	XMFLOAT4 EmissiveFactor{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
	float MetallicFactor{ 1.0f };
	float RoughnessFactor{ 1.0f };
	float AlphaCutoff{ 0.5f };
	Texture* BaseColorTexture{ nullptr };
	Texture* NormalTexture{ nullptr };

	void Release()
	{
		//delete BaseColorTexture;
		//delete NormalTexture;
	}
};
