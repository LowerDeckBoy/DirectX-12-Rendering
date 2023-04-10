#include "../../Core/Device.hpp"
#include "../Camera.hpp"
#include "assimp_Model.hpp"
#include <imgui.h>

void assimp_Model::Create(Device* pDevice, std::string_view Filepath)
{
	if (!Import(pDevice, Filepath)) 
	{
		throw std::exception();
	}

	m_VertexBuffer = std::make_unique<VertexBuffer<Vertex>>(pDevice, m_Vertices);
	m_IndexBuffer = std::make_unique<IndexBuffer>(pDevice, m_Indices);
	m_ConstBuffer = std::make_unique<ConstantBuffer<cbPerObject>>(pDevice, &m_cbData);
	m_cbCamera = std::make_unique<ConstantBuffer<cbCamera>>(pDevice, &m_cbCameraData);
	m_cbLight = std::make_unique<ConstantBuffer<cbMaterial>>(pDevice, &m_cbLightData);

}

void assimp_Model::Draw(Camera* pCamera)
{
	m_Device->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Device->GetCommandList()->IASetVertexBuffers(0, 1, &m_VertexBuffer->GetBufferView());

	for (size_t i = 0; i < m_Meshes.size(); i++)
	{
		// World transformations
		UpdateWorld();
		// Constant buffers
		m_ConstBuffer->Update({ XMMatrixTranspose(m_Meshes.at(i)->Matrix * m_WorldMatrix * pCamera->GetViewProjection()), 
								XMMatrixTranspose(m_WorldMatrix) });
		m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(0, m_ConstBuffer->GetBuffer()->GetGPUVirtualAddress());
		m_cbCamera->Update({ XMFLOAT3(XMVectorGetX(pCamera->GetPosition()), 
									  XMVectorGetY(pCamera->GetPosition()), 
									  XMVectorGetZ(pCamera->GetPosition()))});
		m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(1, m_cbCamera->GetBuffer()->GetGPUVirtualAddress());

		//m_cbLight->Update(m_cbLightData);
		m_cbLight->Update({ *(XMFLOAT4*)m_Ambient.data(),
							*(XMFLOAT3*)m_Diffuse.data(), 
							*(XMFLOAT3*)m_Specular.data(), 
							m_SpecularIntensity,
							*(XMFLOAT3*)m_Direction.data()});
		m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(2, m_cbLight->GetBuffer()->GetGPUVirtualAddress());
		
		// Texturing
		if (m_Materials.at(i)->BaseColorTexture)
			m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(3, m_Materials.at(i)->BaseColorTexture->m_Descriptor.GetGPU());
		if (m_Materials.at(i)->NormalTexture)
			m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(4, m_Materials.at(i)->NormalTexture->m_Descriptor.GetGPU());
		if (m_Materials.at(i)->MetallicRoughnessTexture)
			m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(5, m_Materials.at(i)->MetallicRoughnessTexture->m_Descriptor.GetGPU());
		if (m_Materials.at(i)->EmissiveTexture)
			m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(6, m_Materials.at(i)->EmissiveTexture->m_Descriptor.GetGPU());

		if (m_Meshes.at(i)->bHasIndices)
		{
			m_Device->GetCommandList()->IASetIndexBuffer(&m_IndexBuffer->GetBufferView());
			m_Device->GetCommandList()->DrawIndexedInstanced(m_Meshes.at(i)->IndexCount, 1, 
															 m_Meshes.at(i)->FirstIndexLocation,
															 m_Meshes.at(i)->BaseVertexLocation, 0);
		}
		else
		{
			m_Device->GetCommandList()->DrawInstanced(m_Meshes.at(i)->VertexCount, 1, m_Meshes.at(i)->StartVertexLocation, 0);
		}
	}
}

void assimp_Model::DrawGUI()
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
	}
	ImGui::End();
}

void assimp_Model::UpdateWorld()
{
	m_WorldMatrix = XMMatrixIdentity();
	m_WorldMatrix = XMMatrixScalingFromVector(m_Scale) * XMMatrixRotationRollPitchYawFromVector(m_Rotation) * XMMatrixTranslationFromVector(m_Translation);
}
