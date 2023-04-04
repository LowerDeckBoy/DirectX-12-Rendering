#include "../../Core/Device.hpp"
#include "assimp_Model.hpp"
#include "../Camera.hpp"
#include <imgui.h>

void assimp_Model::Create(Device* pDevice, std::string_view Filepath)
{
	if (!Import(pDevice, Filepath)) 
	{
		throw std::exception();
	}

	m_VertexBuffer = std::make_unique<VertexBuffer<Vertex>>();
	m_VertexBuffer->Create(pDevice, m_Vertices);

	m_IndexBuffer = std::make_unique<IndexBuffer>();
	m_IndexBuffer->Create(pDevice, m_Indices);

	m_ConstBuffer = std::make_unique<ConstantBuffer<cbPerObject>>();
	m_ConstBuffer->Create(pDevice, &m_cbData);

}

void assimp_Model::Draw(Camera* pCamera)
{
	m_Device->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Device->GetCommandList()->IASetVertexBuffers(0, 1, &m_VertexBuffer->GetBufferView());

	for (size_t i = 0; i < m_Meshes.size(); i++)
	{
		UpdateWorld();
		m_cbData.WVP = XMMatrixTranspose(m_Meshes.at(i)->Matrix * m_WorldMatrix * pCamera->GetViewProjection());
		std::memcpy(m_ConstBuffer->pDataBegin, &m_cbData, sizeof(m_cbData));
		m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(0, m_ConstBuffer->GetBuffer()->GetGPUVirtualAddress());

		if (m_Materials.at(i)->BaseColorTexture)
			m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(1, m_Materials.at(i)->BaseColorTexture->m_Descriptor.m_gpuHandle);
		if (m_Materials.at(i)->NormalTexture)
			m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(2, m_Materials.at(i)->NormalTexture->m_Descriptor.m_gpuHandle);
		if (m_Materials.at(i)->MetallicRoughnessTexture)
			m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(3, m_Materials.at(i)->MetallicRoughnessTexture->m_Descriptor.m_gpuHandle);


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
	{
		ImGui::Begin("Model");

		if (ImGui::DragFloat3("Translation", m_Translations.data()))
		{
			m_Translation.m128_f32[0] = m_Translations.at(0);
			m_Translation.m128_f32[1] = m_Translations.at(1);
			m_Translation.m128_f32[2] = m_Translations.at(2);
		}

		if (ImGui::DragFloat3("Rotation", m_Rotations.data(), 0.5f, 0.0f, 360.0f))
		{
			m_Rotation.m128_f32[0] = XMConvertToRadians(m_Rotations.at(0));
			m_Rotation.m128_f32[1] = XMConvertToRadians(m_Rotations.at(1));
			m_Rotation.m128_f32[2] = XMConvertToRadians(m_Rotations.at(2));
		}

		if (ImGui::DragFloat3("Scale", m_Scales.data()))
		{
			m_Scale.m128_f32[0] = m_Scales.at(0);
			m_Scale.m128_f32[1] = m_Scales.at(1);
			m_Scale.m128_f32[2] = m_Scales.at(2);
		}

		ImGui::End();
	}
}

void assimp_Model::UpdateWorld()
{
	m_WorldMatrix = XMMatrixIdentity();
	m_WorldMatrix = XMMatrixScalingFromVector(m_Scale) * XMMatrixRotationRollPitchYawFromVector(m_Rotation) * XMMatrixTranslationFromVector(m_Translation);
}
