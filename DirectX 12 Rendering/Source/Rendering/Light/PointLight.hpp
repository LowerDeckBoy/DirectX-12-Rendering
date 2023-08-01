#pragma once
#include "../../Core/DeviceContext.hpp"
#include "../../Graphics/ConstantBuffer.hpp"

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

