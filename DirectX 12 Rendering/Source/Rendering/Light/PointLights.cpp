#include "PointLights.hpp"
#include <imgui.h>

void PointLights::Create(DeviceContext* pDevice)
{
	m_DeviceCtx = pDevice;

	m_cbPointLights = std::make_unique<ConstantBuffer<cbLights>>(m_DeviceCtx, &m_cbPointLightsData);
	//test
	m_cbLightData = std::make_unique<ConstantBuffer<cbLightShadows>>(m_DeviceCtx, &m_cbShadowsData);

	SetupLights();
}

void PointLights::SetupLights()
{
	m_LightPositions.resize(4);
	m_LightPositionsFloat.resize(4);

	m_LightColors.resize(4);
	m_LightColorsFloat.resize(4);

	m_LightPositions = {
		   XMFLOAT4(-9.0f, +1.0f, 0.0f, 5.0f),
		   XMFLOAT4(+0.0f, +1.0f, 0.0f, 5.0f),
		   XMFLOAT4(+5.0f, +1.0f, 0.0f, 5.0f),
		   XMFLOAT4(+9.0f, +1.0f, 0.0f, 5.0f)
	};

	m_LightPositionsFloat.at(0) = { -9.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(1) = { +0.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(2) = { +5.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(3) = { +9.0f, +1.0f, 0.0f, 25.0f };

	for (size_t i = 0; i < m_LightColors.size(); i++)
	{
		m_LightColors.at(i) = {1.0f, 1.0f, 1.0f, 1.0f};
		m_LightColorsFloat.at(i) = {1.0f, 1.0f, 1.0f, 1.0f};
	}

	m_PointLights.resize(4);

}

void PointLights::UpdateLights()
{
	
	for (size_t i = 0; i < 4; i++)
	{
		m_cbPointLightsData.LightPositions.at(i) = XMFLOAT4(m_LightPositionsFloat.at(i).at(0), m_LightPositionsFloat.at(i).at(1), m_LightPositionsFloat.at(i).at(2), m_LightPositionsFloat.at(i).at(3));

		m_cbPointLightsData.LightColors.at(i) = XMFLOAT4(m_LightColorsFloat.at(i).at(0), m_LightColorsFloat.at(i).at(1), m_LightColorsFloat.at(i).at(2), m_LightColorsFloat.at(i).at(3));
	}

	m_cbPointLights->Update(m_cbPointLightsData, m_DeviceCtx->FRAME_INDEX);
	
	const auto& l{ m_cbPointLightsData.LightPositions.at(0) };
	m_Position = XMVectorSet(l.x, l.y, l.z, 0.0f);
	XMVECTOR target{ XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f) };
	m_View = XMMatrixLookAtLH(m_Position, m_Position - target, m_Up);
	m_Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), 1.0f, 0.1f, 100.0f);

	m_cbShadowsData.ViewProjection = XMMatrixTranspose(m_View * m_Projection);
	m_cbShadowsData.LightPosition = m_cbPointLightsData.LightPositions.at(0);
	m_cbShadowsData.LightColor = m_cbPointLightsData.LightColors.at(0);
	m_cbLightData->Update(m_cbShadowsData, m_DeviceCtx->FRAME_INDEX);

	//TEST
	/*
	for (size_t i = 0; i < m_PointLights.size(); i++)
	{
		m_PointLights.at(i).Color = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		m_PointLights.at(i).Position = XMVectorSet(1.0f - i * 3.0f, 1.0f, 1.0f, 25.0f);

		m_Position = XMVectorSet(
			m_PointLights.at(i).Position.m128_f32[0], 
			m_PointLights.at(i).Position.m128_f32[1],
			m_PointLights.at(i).Position.m128_f32[2], 
			0.0f
		);

		XMVECTOR target{ XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
		//m_View = XMMatrixLookAtLH(m_Position, m_Position - target, m_Up);
		m_View = XMMatrixLookAtLH(m_Position, target, m_Up);
		m_Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), 1.0f, 1.0f, 100.0f);

		m_PointLights.at(i).ViewProjection = XMMatrixTranspose(XMMatrixIdentity() * m_View * m_Projection);

		m_cbShadowsData.LightPosition[i] = m_PointLights.at(i).Position;
		m_cbShadowsData.LightColor[i] = m_PointLights.at(i).Color;
		m_cbShadowsData.ViewProjection[i] = m_PointLights.at(i).ViewProjection;
	}
	//m_cbLightData->Update(m_cbShadowsData, m_DeviceCtx->FRAME_INDEX);
	*/
}

void PointLights::ResetLights()
{
	m_LightPositionsFloat.at(0) = { -9.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(1) = { +0.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(2) = { +5.0f, +1.0f, 0.0f, 25.0f };
	m_LightPositionsFloat.at(3) = { +9.0f, +1.0f, 0.0f, 25.0f };

	for (auto& color : m_LightColorsFloat)
		color = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void PointLights::DrawGUI()
{
	ImGui::Begin("Point Lights");
	ImGui::Text("Positions");
	for (size_t i = 0; i < m_Count; i++)
	{
		auto label{ "Light #" + std::to_string(i) };
		ImGui::DragFloat4(label.c_str(), m_LightPositionsFloat.at(i).data());
	}

	//ImGui::DragFloat4("Light 1", m_LightPositionsFloat.at(0).data());
	//ImGui::DragFloat4("Light 2", m_LightPositionsFloat.at(1).data());
	//ImGui::DragFloat4("Light 3", m_LightPositionsFloat.at(2).data());
	//ImGui::DragFloat4("Light 4", m_LightPositionsFloat.at(3).data());
	//ImGui::Text("Colors");
	for (size_t i = 0; i < m_Count; i++)
	{
		auto label{ "Color #" + std::to_string(i) };
		ImGui::ColorEdit4(label.c_str(), m_LightColorsFloat.at(i).data());
	}
	//ImGui::ColorEdit4("Point Light 1", m_LightColorsFloat.at(0).data());
	//ImGui::ColorEdit4("Point Light 2", m_LightColorsFloat.at(1).data());
	//ImGui::ColorEdit4("Point Light 3", m_LightColorsFloat.at(2).data());
	//ImGui::ColorEdit4("Point Light 4", m_LightColorsFloat.at(3).data());
	if (ImGui::Button("Reset Lights"))
	{
		ResetLights();
	}

	/*
	if (ImGui::Button("Add"))
	{
		m_LightPositions.push_back(XMFLOAT4(-9.0f, +1.0f, 0.0f, 25.0f));
		m_LightPositionsFloat.push_back({ -9.0f, +1.0f, 0.0f, 25.0f });
		m_LightColors.push_back(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		m_LightColorsFloat.push_back({ 1.0f, 1.0f, 1.0f, 1.0f });

		m_Count++;
		//m_cbPointLightsData.LightsCount = m_Count;
		//m_cbPointLights->Update(m_cbPointLightsData, m_DeviceCtx->FRAME_INDEX);

		//m_cbP
	}

	//ImGui::Text("Count: %u", m_cbPointLightsData.LightsCount);
	*/
	ImGui::End();
}
