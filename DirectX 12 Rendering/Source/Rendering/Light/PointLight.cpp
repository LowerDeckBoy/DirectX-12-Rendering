#include "PointLight.hpp"
#include <imgui.h>

void PointLight::Create(DeviceContext* pDevice)
{
	m_DeviceCtx = pDevice;

	m_cbPointLights = std::make_unique<ConstantBuffer<cbLights>>(m_DeviceCtx, &m_cbPointLightsData);

	SetupLights();
}

void PointLight::SetupLights()
{
	m_LightPositions = {
		   XMFLOAT4(-9.0f, +1.0f, 0.0f, 5.0f),
		   XMFLOAT4(+0.0f, +1.0f, 0.0f, 5.0f),
		   XMFLOAT4(+5.0f, +1.0f, 0.0f, 5.0f),
		   XMFLOAT4(+9.0f, +1.0f, 0.0f, 5.0f)
	};

	m_LightColors = {
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)
	};

	m_LightPositionsFloat.at(0) = { -9.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(1) = { +0.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(2) = { +5.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(3) = { +9.0f, +1.0f, 0.0f, 25.0f };

	m_LightColorsFloat.at(0) = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_LightColorsFloat.at(1) = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_LightColorsFloat.at(2) = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_LightColorsFloat.at(3) = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void PointLight::UpdateLights()
{
	for (size_t i = 0; i < 4; i++)
	{
		m_cbPointLightsData.LightPositions.at(i) = XMFLOAT4(m_LightPositionsFloat.at(i).at(0), m_LightPositionsFloat.at(i).at(1), m_LightPositionsFloat.at(i).at(2), m_LightPositionsFloat.at(i).at(3));

		m_cbPointLightsData.LightColors.at(i) = XMFLOAT4(m_LightColorsFloat.at(i).at(0), m_LightColorsFloat.at(i).at(1), m_LightColorsFloat.at(i).at(2), m_LightColorsFloat.at(i).at(3));
	}
	/*
	m_cbPointLightsData.LightPositions.at(0) = XMFLOAT4(m_LightPositionsFloat.at(0).at(0), m_LightPositionsFloat.at(0).at(1), m_LightPositionsFloat.at(0).at(2), m_LightPositionsFloat.at(0).at(3));
	m_cbPointLightsData.LightPositions.at(1) = XMFLOAT4(m_LightPositionsFloat.at(1).at(0), m_LightPositionsFloat.at(1).at(1), m_LightPositionsFloat.at(1).at(2), m_LightPositionsFloat.at(1).at(3));
	m_cbPointLightsData.LightPositions.at(2) = XMFLOAT4(m_LightPositionsFloat.at(2).at(0), m_LightPositionsFloat.at(2).at(1), m_LightPositionsFloat.at(2).at(2), m_LightPositionsFloat.at(2).at(3));
	m_cbPointLightsData.LightPositions.at(3) = XMFLOAT4(m_LightPositionsFloat.at(3).at(0), m_LightPositionsFloat.at(3).at(1), m_LightPositionsFloat.at(3).at(2), m_LightPositionsFloat.at(3).at(3));

	m_cbPointLightsData.LightColors.at(0) = XMFLOAT4(m_LightColorsFloat.at(0).at(0), m_LightColorsFloat.at(0).at(1), m_LightColorsFloat.at(0).at(2), m_LightColorsFloat.at(0).at(3));
	m_cbPointLightsData.LightColors.at(1) = XMFLOAT4(m_LightColorsFloat.at(1).at(0), m_LightColorsFloat.at(1).at(1), m_LightColorsFloat.at(1).at(2), m_LightColorsFloat.at(1).at(3));
	m_cbPointLightsData.LightColors.at(2) = XMFLOAT4(m_LightColorsFloat.at(2).at(0), m_LightColorsFloat.at(2).at(1), m_LightColorsFloat.at(2).at(2), m_LightColorsFloat.at(2).at(3));
	m_cbPointLightsData.LightColors.at(3) = XMFLOAT4(m_LightColorsFloat.at(3).at(0), m_LightColorsFloat.at(3).at(1), m_LightColorsFloat.at(3).at(2), m_LightColorsFloat.at(3).at(3));
	*/

	m_cbPointLights->Update(m_cbPointLightsData, m_DeviceCtx->FRAME_INDEX);
}

void PointLight::ResetLights()
{
	m_LightPositionsFloat.at(0) = { -9.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(1) = { +0.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(2) = { +5.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(3) = { +9.0f, +1.0f, 0.0f, 25.0f };

	m_LightColorsFloat.at(0) = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_LightColorsFloat.at(1) = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_LightColorsFloat.at(2) = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_LightColorsFloat.at(3) = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void PointLight::DrawGUI()
{
	ImGui::Begin("Point Lights");
	ImGui::Text("Positions");
	ImGui::DragFloat4("Point Light 1", m_LightPositionsFloat.at(0).data());
	ImGui::DragFloat4("Point Light 2", m_LightPositionsFloat.at(1).data());
	ImGui::DragFloat4("Point Light 3", m_LightPositionsFloat.at(2).data());
	ImGui::DragFloat4("Point Light 4", m_LightPositionsFloat.at(3).data());
	ImGui::Text("Colors");
	ImGui::ColorEdit4("Point Light 1", m_LightColorsFloat.at(0).data());
	ImGui::ColorEdit4("Point Light 2", m_LightColorsFloat.at(1).data());
	ImGui::ColorEdit4("Point Light 3", m_LightColorsFloat.at(2).data());
	ImGui::ColorEdit4("Point Light 4", m_LightColorsFloat.at(3).data());
	if (ImGui::Button("Reset Lights"))
	{
		ResetLights();
	}
	ImGui::End();
}
