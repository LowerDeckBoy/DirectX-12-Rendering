#pragma once
#include "../../Core/DeviceContext.hpp"
#include "../../Graphics/ConstantBuffer.hpp"
//#include <DirectXMath.h>
//#include "../Model/Model.hpp"

/*
struct cbPointLight
{
	XMFLOAT4 Ambient{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
	alignas(16) XMFLOAT3 Diffuse{ XMFLOAT3(1.0f, 1.0f, 1.0f) };
	alignas(16) XMFLOAT3 Position{ XMFLOAT3(0.0f, 0.0f, 0.0f) };
	float Attenuation;
	float Range;
};
*/

class PointLight
{
public:
	PointLight() {}
	
	void Create(DeviceContext* pDevice);

	void SetupLights();
	void UpdateLights();
	void ResetLights();

	void DrawGUI();

	std::unique_ptr<ConstantBuffer<cbLights>> m_cbPointLights;
	cbLights m_cbPointLightsData{};

	bool bCastShadows{ false };

private:
	DeviceContext* m_DeviceCtx{ nullptr };

	std::array<XMFLOAT4, 4>				m_LightPositions;
	std::array<std::array<float, 4>, 4> m_LightPositionsFloat;
	std::array<XMFLOAT4, 4>				m_LightColors;
	std::array<std::array<float, 4>, 4> m_LightColorsFloat;

};

