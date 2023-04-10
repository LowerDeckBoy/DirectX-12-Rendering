#include "../../Core/Device.hpp"
#include "../Camera.hpp"
#include "cglTF_Model.hpp"
#include <imgui.h>

cglTF_Model::cglTF_Model(Device* pDevice, std::string_view Filepath) 
	: cglTF_Loader(pDevice, Filepath)
{
	//Create(pDevice, Filepath);
}

void cglTF_Model::Create(Device* pDevice, std::string_view Filepath)
{
	if (bInitialized)
	{
		::OutputDebugStringA("[glTF] Model is already created!\n");
		return;
	}

	if (!Import(pDevice, Filepath))
	{
		throw std::exception();
	}

	m_VertexBuffer = std::make_unique<VertexBuffer<Vertex>>();
	m_VertexBuffer->Create(m_Device, m_Vertices);
	//m_Vertices.clear();
	//m_Vertices.shrink_to_fit();
	
	m_IndexBuffer = std::make_unique<IndexBuffer>();
	m_IndexBuffer->Create(m_Device, m_Indices);
	//m_Indices.clear();
	//m_Indices.shrink_to_fit();

	//m_ConstantBuffer = std::make_unique<ConstantBuffer<cbPerObject>>();
	//m_ConstantBuffer->Create(m_Device, &m_cbData);
	m_ConstantBuffers.resize(2);
	m_cbDatas.resize(2);
	for (uint32_t i = 0; i < 2; i++)
	{
		m_ConstantBuffers.at(i).Create(m_Device, &m_cbData);
	}

	bInitialized = true;
}

void cglTF_Model::Draw(uint32_t CurrentFrame, Camera* pCamera)
{
	m_Device->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Device->GetCommandList()->IASetVertexBuffers(0, 1, &m_VertexBuffer->GetBufferView());

	for (auto& node : m_Nodes)
	{
		DrawNode(CurrentFrame, pCamera, node);
	}
}

void cglTF_Model::DrawNode(uint32_t CurrentFrame, Camera* pCamera, cglTF::Node* pNode)
{
	if (pNode->Mesh)
	{
		UpdateWorld();

		//m_cbData.WVP   = XMMatrixTranspose(pNode->Mesh->Matrix * m_WorldMatrix * pCamera->GetViewProjection());
		//m_cbData.World = XMMatrixTranspose(m_WorldMatrix);
		//std::memcpy(m_ConstantBuffer->pDataBegin, &m_cbData, sizeof(m_cbData));
		//m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(0, m_ConstantBuffer->GetBuffer()->GetGPUVirtualAddress());
		//pNode->Mesh->Matrix
		m_cbDatas.at(CurrentFrame).WVP = XMMatrixTranspose(pNode->Mesh->Matrix * m_WorldMatrix * pCamera->GetViewProjection());
		m_cbDatas.at(CurrentFrame).World = XMMatrixTranspose(m_WorldMatrix);
		std::memcpy(m_ConstantBuffers.at(CurrentFrame).pDataBegin, &m_cbDatas.at(CurrentFrame), sizeof(cbPerObject));
		m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(0, m_ConstantBuffers.at(CurrentFrame).GetBuffer()->GetGPUVirtualAddress());

		for (auto* primitive : pNode->Mesh->Primitives)
		{
			if (primitive->Material.BaseColorTexture)
				m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(1, primitive->Material.BaseColorTexture->m_Descriptor.GetGPU());
			if (primitive->Material.NormalTexture)
				m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(2, primitive->Material.NormalTexture->m_Descriptor.GetGPU());
			if (primitive->Material.MetallicTexture)
				m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(3, primitive->Material.MetallicTexture->m_Descriptor.GetGPU());

			if (primitive->bHasIndices)
			{
				m_Device->GetCommandList()->IASetIndexBuffer(&m_IndexBuffer->GetBufferView());
				m_Device->GetCommandList()->DrawIndexedInstanced(primitive->IndexCount, 1,
																 primitive->FirstIndexLocation,
																 primitive->BaseVertexLocation, 0);
			}
			else
			{
				m_Device->GetCommandList()->DrawInstanced(primitive->VertexCount, 1, primitive->StartVertexLocation, 0);
			}
		}
	}
		
	for (auto child : pNode->Children)
		DrawNode(CurrentFrame, pCamera, child);

}

void cglTF_Model::DrawGUI()
{
	{
		ImGui::Begin("Model");

		if (ImGui::DragFloat3("Translation", m_Translations.data()))
		{
			m_Translation = XMVectorSet(m_Translations.at(0), 
										m_Translations.at(1), 
										m_Translations.at(2), 
										0.0f);
		}

		if (ImGui::DragFloat3("Rotation", m_Rotations.data(), 0.5f, 0.0f, 360.0f))
		{
			m_Rotation = XMVectorSet(XMConvertToRadians(m_Rotations.at(0)),
									 XMConvertToRadians(m_Rotations.at(1)),
									 XMConvertToRadians(m_Rotations.at(2)),
									 0.0f);

		}

		if (ImGui::DragFloat3("Scale", m_Scales.data()))
		{
			m_Scale = XMVectorSet(m_Scales.at(0), 
								  m_Scales.at(1), 
								  m_Scales.at(2), 
								  0.0f);
		}

		ImGui::End();
	}
}

void cglTF_Model::UpdateWorld()
{
	m_WorldMatrix = XMMatrixIdentity();
	m_WorldMatrix = XMMatrixScalingFromVector(m_Scale) * XMMatrixRotationRollPitchYawFromVector(m_Rotation) * XMMatrixTranslationFromVector(m_Translation);
}
