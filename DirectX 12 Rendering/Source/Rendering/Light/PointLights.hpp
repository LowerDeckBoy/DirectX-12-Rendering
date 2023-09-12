#pragma once
#include "../../Core/DeviceContext.hpp"
#include "../../Graphics/Buffer/ConstantBuffer.hpp"


/*
struct cbLightShadows
{
	XMMATRIX ViewProjection{ XMMatrixIdentity() };
	XMFLOAT4 LightPosition { XMFLOAT4(-9.0f, 0.0f, 10.0f, 0.0f) };
	XMFLOAT4 LightColor    { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
};
*/
// Temporal
struct cbLightShadows
{
	XMMATRIX ViewProjection;
	XMFLOAT4 LightPosition[4];
	XMFLOAT4 LightColor[4];
};
struct PointLight
{
	// xyz -> position, w -> light intensity
	XMVECTOR Position		{ XMVectorSet(0.0f, 0.0f, 0.0f, 25.0f) };
	XMVECTOR Color			{ XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f) };
	XMVECTOR Target			{ XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f) };
	XMMATRIX ViewProjection { XMMatrixIdentity() };
};

class PointLights
{
public:
	PointLights() {}
	
	void Create(DeviceContext* pDevice);

	void SetupLights();
	void UpdateLights();
	void ResetLights();

	void DrawGUI();

	std::unique_ptr<ConstantBuffer<cbLights>> m_cbPointLights;
	cbLights m_cbPointLightsData{};
	std::unique_ptr<ConstantBuffer<cbLightShadows>> m_cbLightData;
	cbLightShadows m_cbShadowsData{};

	bool bCastShadows{ false };

private:
	DeviceContext* m_DeviceCtx{ nullptr };

	std::vector<XMFLOAT4>				m_LightPositions{};
	std::vector<std::array<float, 4>>   m_LightPositionsFloat{};
	std::vector<XMFLOAT4>				m_LightColors{};
	std::vector<std::array<float, 4>>	m_LightColorsFloat{};

	uint32_t m_Count{ 4 };

	std::vector<PointLight> m_PointLights;

	XMVECTOR m_Position{ XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
	XMVECTOR m_Target{ XMVECTOR() };
	XMVECTOR m_Up{ XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };
	XMMATRIX m_View{ XMMatrixIdentity() };
	XMMATRIX m_Projection{ XMMatrixIdentity() };

};

