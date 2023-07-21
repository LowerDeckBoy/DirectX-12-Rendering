#pragma once
#include "../../Core/DeviceContext.hpp"
#include <DirectXMath.h>
#include "../Model/Model.hpp"

struct cbPointLight
{
	XMFLOAT4 Ambient{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
	alignas(16) XMFLOAT3 Diffuse{ XMFLOAT3(1.0f, 1.0f, 1.0f) };
	alignas(16) XMFLOAT3 Position{ XMFLOAT3(0.0f, 0.0f, 0.0f) };
	float Attenuation;
	float Range;
};

class PointLight
{
public:
	PointLight() {}
	
	void Create(DeviceContext* pDevice, const std::string_view& Filepath);

private:
	DeviceContext* m_Device{ nullptr };

	float m_Radius;
	// World position
	XMMATRIX m_WorldMatrix{ XMMatrixIdentity() };
	XMVECTOR m_Translate;
	XMVECTOR m_Scale;
	std::array<float, 3> m_Position{ 0.0f, 0.0f, 0.0f };
	std::array<float, 3> m_Scales{ 0.1f, 0.1f, 0.1f };


};

