#include "../../Core/DeviceContext.hpp"
#include "../Camera.hpp"
#include "Model.hpp"
#include <imgui.h>


Model::Model(DeviceContext* pDevice, std::string_view Filepath, const std::string& ModelName)
{
	Create(pDevice, Filepath);
	m_ModelName = ModelName;
}

Model::~Model()
{
	Release();
}

void Model::Create(DeviceContext* pDevice, std::string_view Filepath)
{
	if (!Import(pDevice, Filepath)) 
	{
		throw std::exception();
	}

	m_VertexBuffer	= std::make_unique<VertexBuffer>(pDevice, BufferData(m_Vertices.data(), m_Vertices.size(), sizeof(m_Vertices.at(0)) * m_Vertices.size(), sizeof(m_Vertices.at(0))), BufferDesc(), true);
	m_IndexBuffer	= std::make_unique<IndexBuffer>(pDevice, BufferData(m_Indices.data(), m_Indices.size(), sizeof(uint32_t) * m_Indices.size(), sizeof(uint32_t)), BufferDesc(), true);
	m_cbPerObject	= std::make_unique<ConstantBuffer<cbPerObject>>(pDevice, &m_cbPerObjectData);
	m_cbCamera		= std::make_unique<ConstantBuffer<cbCamera>>(pDevice, &m_cbCameraData);
	m_cbMaterial	= std::make_unique<ConstantBuffer<cbMaterial>>(pDevice, &m_cbMaterialData);

	m_Vertices.clear();
	m_Vertices.shrink_to_fit();
	m_Indices.clear();
	m_Indices.shrink_to_fit();

	UpdateWorld();
}

void Model::Draw(Camera* pCamera)
{
	m_Device->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Device->GetCommandList()->IASetVertexBuffers(0, 1, &m_VertexBuffer->View);

	for (size_t i = 0; i < m_Meshes.size(); i++)
	{
		const auto frameIndex{ m_Device->FRAME_INDEX };
		// World transformations
		//UpdateWorld();
		// Constant buffers
		m_cbPerObject->Update({ XMMatrixTranspose(m_Meshes.at(i)->Matrix * m_WorldMatrix * pCamera->GetViewProjection()),
								XMMatrixTranspose(m_WorldMatrix) }, frameIndex);
		m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(0, m_cbPerObject->GetBuffer(frameIndex)->GetGPUVirtualAddress());

		auto currentMaterial{ m_Materials.at(i) };

		m_cbMaterial->Update({
							pCamera->GetPositionFloat(),
							currentMaterial->BaseColorFactor,
							currentMaterial->EmissiveFactor,
							currentMaterial->MetallicFactor,
							currentMaterial->RoughnessFactor,
							currentMaterial->AlphaCutoff,
							currentMaterial->bDoubleSided
			}, frameIndex);

		m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(2, m_cbMaterial->GetBuffer(frameIndex)->GetGPUVirtualAddress());
		const model::MaterialIndices indices{ currentMaterial->BaseColorIndex, 
											currentMaterial->NormalIndex, 
											currentMaterial->MetallicRoughnessIndex, 
											currentMaterial->EmissiveIndex };
		m_Device->GetCommandList()->SetGraphicsRoot32BitConstants(5, sizeof(indices) / 4, &indices, 0);

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
	ImGui::Begin(m_ModelName.c_str());
	// Transforms
	{
		if (ImGui::DragFloat3("Translation", m_Translations.data()))
		{
			m_Translation = XMVectorSet(m_Translations.at(0), m_Translations.at(1), m_Translations.at(2), 0.0f);
			UpdateWorld();
		}

		if (ImGui::DragFloat3("Rotation", m_Rotations.data(), 0.5f))
		{
			m_Rotation = XMVectorSet(XMConvertToRadians(m_Rotations.at(0)),
									 XMConvertToRadians(m_Rotations.at(1)),
									 XMConvertToRadians(m_Rotations.at(2)),
									 0.0f);
			UpdateWorld();
		}

		if (ImGui::DragFloat3("Scale", m_Scales.data(), 0.1f))
		{
			m_Scale = XMVectorSet(m_Scales.at(0), m_Scales.at(1), m_Scales.at(2), 0.0f);
			UpdateWorld();
		}

		if (ImGui::Button("Reset"))
		{
			m_Translations	= { 0.0f, 0.0f, 0.0f };
			m_Rotations		= { 0.0f, 0.0f, 0.0f };
			m_Scales		= { 1.0f, 1.0f, 1.0f };

			m_Translation	= XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			m_Rotation		= XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			m_Scale			= XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
			UpdateWorld();
		}
	}
	
	ImGui::End();
}

void Model::Release()
{
	for (auto& mesh : m_Meshes)
		delete mesh;

	for (auto& material : m_Materials)
		delete material;
}

void Model::UpdateWorld()
{
	m_WorldMatrix = XMMatrixIdentity();
	m_WorldMatrix = XMMatrixScalingFromVector(m_Scale) * XMMatrixRotationRollPitchYawFromVector(m_Rotation) * XMMatrixTranslationFromVector(m_Translation);
}
