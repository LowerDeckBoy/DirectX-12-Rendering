#include "../../Core/Device.hpp"
#include "../Camera.hpp"
#include "Model.hpp"
#include <imgui.h>
#include "../../Graphics/Skybox.hpp"

Model::Model(Device* pDevice, std::string_view Filepath)
{
	Create(pDevice, Filepath);
}

Model::~Model()
{
	Release();
}

void Model::Create(Device* pDevice, std::string_view Filepath)
{
	if (!Import(pDevice, Filepath)) 
	{
		throw std::exception();
	}

	m_VertexBuffer	= std::make_unique<VertexBuffer>(pDevice, BufferData(m_Vertices.data(), m_Vertices.size(), sizeof(m_Vertices.at(0)) * m_Vertices.size()), BufferDesc());
	m_IndexBuffer	= std::make_unique<IndexBuffer>(pDevice, BufferData(m_Indices.data(), m_Indices.size(), sizeof(uint32_t) * m_Indices.size()), BufferDesc());
	m_ConstBuffer	= std::make_unique<ConstantBuffer<cbPerObject>>(pDevice, &m_cbData);
	m_cbCamera		= std::make_unique<ConstantBuffer<cbCamera>>(pDevice, &m_cbCameraData);
	m_cbMaterial	= std::make_unique<ConstantBuffer<cbMaterial>>(pDevice, &m_cbMaterialData);

	// TEST 
	DoLights();
	m_cbPointLights = std::make_unique<ConstantBuffer<cbLights>>(pDevice, &m_cbPointLightsData);
}

void Model::Draw(Camera* pCamera)
{
	m_Device->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Device->GetCommandList()->IASetVertexBuffers(0, 1, &m_VertexBuffer->View);

	for (size_t i = 0; i < m_Meshes.size(); i++)
	{
		auto frameIndex{ m_Device->m_FrameIndex };
		// World transformations
		UpdateWorld();
		// Constant buffers
		m_ConstBuffer->Update({ XMMatrixTranspose(m_Meshes.at(i)->Matrix * m_WorldMatrix * pCamera->GetViewProjection()), 
								XMMatrixTranspose(m_WorldMatrix) }, frameIndex);
		m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(0, m_ConstBuffer->GetBuffer(frameIndex)->GetGPUVirtualAddress());

		m_cbCamera->Update({ XMFLOAT3(XMVectorGetX(pCamera->GetPosition()), 
									  XMVectorGetY(pCamera->GetPosition()), 
									  XMVectorGetZ(pCamera->GetPosition())) }, 
									  frameIndex);
		m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(1, m_cbCamera->GetBuffer(frameIndex)->GetGPUVirtualAddress());

		auto currentMaterial{ m_Materials.at(i) };

		m_cbMaterial->Update({ *(XMFLOAT4*)m_Ambient.data(),
							*(XMFLOAT4*)m_Diffuse.data(),
							*(XMFLOAT4*)m_Direction.data(),
							pCamera->GetPositionFloat(),
							currentMaterial->BaseColorFactor,
							currentMaterial->EmissiveFactor,
							currentMaterial->MetallicFactor,
							currentMaterial->RoughnessFactor,
							currentMaterial->AlphaCutoff,
							currentMaterial->bDoubleSided,
							currentMaterial->bHasDiffuse,
							currentMaterial->bHasNormal,
							currentMaterial->bHasMetallic,
							currentMaterial->bHasEmissive,
							}, frameIndex);

		//m_cbPointLights->Update({});
		m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(2, m_cbMaterial->GetBuffer(frameIndex)->GetGPUVirtualAddress());
		
		//m_cbPointLightsData.LightPositions = m_LightPositions;
		//m_cbPointLightsData.LightColors = m_LightColors;
		
		UpdateLights();
		m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(3, m_cbPointLights->GetBuffer(frameIndex)->GetGPUVirtualAddress());

		// Texturing
		if (currentMaterial->BaseColorTexture)
			m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(4, currentMaterial->BaseColorTexture->m_Descriptor.GetGPU());
		if (currentMaterial->NormalTexture)
			m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(5, currentMaterial->NormalTexture->m_Descriptor.GetGPU());
		if (currentMaterial->MetallicRoughnessTexture)
			m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(6, currentMaterial->MetallicRoughnessTexture->m_Descriptor.GetGPU());
		if (currentMaterial->EmissiveTexture)
			m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(7, currentMaterial->EmissiveTexture->m_Descriptor.GetGPU());

		if (m_SkyTexture) {
			m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(8, m_SkyTexture->GetTex().m_Descriptor.GetGPU());
		}

		if (m_Meshes.at(i)->bHasIndices)
		{
			m_Device->GetCommandList()->IASetIndexBuffer(&m_IndexBuffer->View);
			m_Device->GetCommandList()->DrawIndexedInstanced(m_Meshes.at(i)->IndexCount, 1, 
															 m_Meshes.at(i)->FirstIndexLocation,
															 m_Meshes.at(i)->BaseVertexLocation, 0);
		}
		else
		{
			m_Device->GetCommandList()->DrawInstanced(m_Meshes.at(i)->VertexCount, 1, m_Meshes.at(i)->StartVertexLocation, 0);
		}
	}

	DrawGUI();
}

void Model::DrawGUI()
{
	ImGui::Begin("Model");
	// Transforms
	{
		if (ImGui::DragFloat3("Translation", m_Translations.data()))
		{
			m_Translation = XMVectorSet(m_Translations.at(0), m_Translations.at(1), m_Translations.at(2), 0.0f);
		}

		if (ImGui::DragFloat3("Rotation", m_Rotations.data(), 0.5f, -360.0f, 360.0f))
		{
			m_Rotation = XMVectorSet(XMConvertToRadians(m_Rotations.at(0)),
									 XMConvertToRadians(m_Rotations.at(1)),
									 XMConvertToRadians(m_Rotations.at(2)),
									 0.0f);
		}

		if (ImGui::DragFloat3("Scale", m_Scales.data(), 0.1f))
		{
			m_Scale = XMVectorSet(m_Scales.at(0), m_Scales.at(1), m_Scales.at(2), 0.0f);
		}

		if (ImGui::Button("Reset"))
		{
			m_Translations	= { 0.0f, 0.0f, 0.0f };
			m_Rotations		= { 0.0f, 0.0f, 0.0f };
			m_Scales		= { 1.0f, 1.0f, 1.0f };

			m_Translation	= XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			m_Rotation		= XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			m_Scale			= XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
		}
	}
	// Shader data
	{
		ImGui::ColorEdit4("Ambient", m_Ambient.data());
		ImGui::ColorEdit3("Diffuse", m_Diffuse.data());
		ImGui::ColorEdit3("Specular", m_Specular.data());
		ImGui::DragFloat("Specular Intensity", &m_SpecularIntensity);
		ImGui::DragFloat3("Direction", m_Direction.data(), 0.25f);

		// Point Lights
		{
			ImGui::Text("Positions");
			ImGui::DragFloat3("Point Light 1", m_LightPositionsFloat.at(0).data());
			ImGui::DragFloat3("Point Light 2", m_LightPositionsFloat.at(1).data());
			ImGui::DragFloat3("Point Light 3", m_LightPositionsFloat.at(2).data());
			ImGui::DragFloat3("Point Light 4", m_LightPositionsFloat.at(3).data());
			ImGui::Text("Colors");
			ImGui::ColorEdit4("Point Light 1", m_LightColorsFloat.at(0).data());
			ImGui::ColorEdit4("Point Light 2", m_LightColorsFloat.at(1).data());
			ImGui::ColorEdit4("Point Light 3", m_LightColorsFloat.at(2).data());
			ImGui::ColorEdit4("Point Light 4", m_LightColorsFloat.at(3).data());
			if (ImGui::Button("Reset Lights"))
			{
				ResetLights();
			}
		}
	}
	ImGui::End();
}

void Model::Release()
{
	m_SkyTexture = nullptr;
}

void Model::UpdateWorld()
{
	m_WorldMatrix = XMMatrixIdentity();
	m_WorldMatrix = XMMatrixScalingFromVector(m_Scale) * XMMatrixRotationRollPitchYawFromVector(m_Rotation) * XMMatrixTranslationFromVector(m_Translation);
}

void Model::DoLights()
{
	m_LightPositions = {
		XMFLOAT4(-9.0f, +1.0f, 0.0f, 1.0f),
		XMFLOAT4(+0.0f, +1.0f, 0.0f, 1.0f),
		XMFLOAT4(+5.0f, +1.0f, 0.0f, 1.0f),
		XMFLOAT4(+9.0f, +1.0f, 0.0f, 1.0f)
	};

	m_LightColors = {
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)
	};

	m_LightPositionsFloat.at(0) = { -9.0f, +1.0f, 0.0f, 1.0f };
	m_LightPositionsFloat.at(1) = { +0.0f, +1.0f, 0.0f, 1.0f };
	m_LightPositionsFloat.at(2) = { +5.0f, +1.0f, 0.0f, 1.0f };
	m_LightPositionsFloat.at(3) = { +9.0f, +1.0f, 0.0f, 1.0f };

	m_LightColorsFloat.at(0) = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_LightColorsFloat.at(1) = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_LightColorsFloat.at(2) = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_LightColorsFloat.at(3) = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void Model::UpdateLights()
{
	m_cbPointLightsData.LightPositions.at(0) = XMFLOAT4(m_LightPositionsFloat.at(0).at(0), m_LightPositionsFloat.at(0).at(1), m_LightPositionsFloat.at(0).at(2), m_LightPositionsFloat.at(0).at(3));
	m_cbPointLightsData.LightPositions.at(1) = XMFLOAT4(m_LightPositionsFloat.at(1).at(0), m_LightPositionsFloat.at(1).at(1), m_LightPositionsFloat.at(1).at(2), m_LightPositionsFloat.at(1).at(3));
	m_cbPointLightsData.LightPositions.at(2) = XMFLOAT4(m_LightPositionsFloat.at(2).at(0), m_LightPositionsFloat.at(2).at(1), m_LightPositionsFloat.at(2).at(2), m_LightPositionsFloat.at(2).at(3));
	m_cbPointLightsData.LightPositions.at(3) = XMFLOAT4(m_LightPositionsFloat.at(3).at(0), m_LightPositionsFloat.at(3).at(1), m_LightPositionsFloat.at(3).at(2), m_LightPositionsFloat.at(3).at(3));

	m_cbPointLightsData.LightColors.at(0) = XMFLOAT4(m_LightColorsFloat.at(0).at(0), m_LightColorsFloat.at(0).at(1), m_LightColorsFloat.at(0).at(2), m_LightColorsFloat.at(0).at(3));
	m_cbPointLightsData.LightColors.at(1) = XMFLOAT4(m_LightColorsFloat.at(1).at(0), m_LightColorsFloat.at(1).at(1), m_LightColorsFloat.at(1).at(2), m_LightColorsFloat.at(1).at(3));
	m_cbPointLightsData.LightColors.at(2) = XMFLOAT4(m_LightColorsFloat.at(2).at(0), m_LightColorsFloat.at(2).at(1), m_LightColorsFloat.at(2).at(2), m_LightColorsFloat.at(2).at(3));
	m_cbPointLightsData.LightColors.at(3) = XMFLOAT4(m_LightColorsFloat.at(3).at(0), m_LightColorsFloat.at(3).at(1), m_LightColorsFloat.at(3).at(2), m_LightColorsFloat.at(3).at(3));

	m_cbPointLights->Update(m_cbPointLightsData, m_Device->m_FrameIndex);
}

void Model::ResetLights()
{
	/*
	m_LightPositionsFloat.at(0) = { -10.0f, +10.0f, -10.0f, 1.0f };
	m_LightPositionsFloat.at(1) = { +10.0f, +10.0f, -10.0f, 1.0f };
	m_LightPositionsFloat.at(2) = { -10.0f, -10.0f, -10.0f, 1.0f };
	m_LightPositionsFloat.at(3) = { +10.0f, +10.0f, -10.0f, 1.0f };
	*/

	m_LightPositionsFloat.at(0) = { -9.0f, +1.0f, 0.0f, 1.0f };
	m_LightPositionsFloat.at(1) = { +0.0f, +1.0f, 0.0f, 1.0f };
	m_LightPositionsFloat.at(2) = { +5.0f, +1.0f, 0.0f, 1.0f };
	m_LightPositionsFloat.at(3) = { +9.0f, +1.0f, 0.0f, 1.0f };

	m_LightColorsFloat.at(0) = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_LightColorsFloat.at(1) = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_LightColorsFloat.at(2) = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_LightColorsFloat.at(3) = { 1.0f, 1.0f, 1.0f, 1.0f };
}
