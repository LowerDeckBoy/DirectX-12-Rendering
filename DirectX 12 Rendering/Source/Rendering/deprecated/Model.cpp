//#ifndef CRT_SECURE_NO_WARNINGS
//#define CRT_SECURE_NO_WARNINGS
//#endif

//#include "ModelImporter.hpp"
#include "../Core/Device.hpp"
#include "Model.hpp"
#include <string>
#include "../Utils/FileHelper.hpp"
#include "../Utils/Utils.hpp"

#include "../Core/Window.hpp"
#include "Camera.hpp"

#include <imgui.h>

using namespace DirectX;

Model::~Model()
{
	Release();
}

void Model::Initialize(Device* pDevice, const std::string& PathToModel)
{
	assert(m_Device = pDevice);

	//ModelImporter* importer{ new ModelImporter() };
	//ModelImporter importer;
	//importer.ImportModel(pDevice, PathToModel, m_Meshes);
	m_Importer = std::make_unique<ModelImporter>();
	m_Importer->ImportModel(pDevice, PathToModel, m_Meshes);

	m_VertexBuffer = std::make_unique<VertexBuffer<Vertex>>();
	m_VertexBuffer->Create(pDevice, m_Importer->m_Vertices);
	
	m_IndexBuffer = std::make_unique<IndexBuffer>();
	m_IndexBuffer->Create(pDevice, m_Importer->m_Indices);

	m_ConstBuffer = std::make_unique<ConstantBuffer<cbPerObject>>();
	m_ConstBuffer->Create(pDevice, &m_cbData);

	
}

void Model::Update()
{	
	
}


void Model::DrawNode(model::Node* pNode, Camera* pCamera)
{
	// && pNode->Mesh->Primitives.size() > 0
	//auto currentFrameIndex{ m_Device->m_FrameIndex };
	UpdateWorld();
	if (pNode->Mesh)
	{
		if (pNode->Mesh->Primitives.size() > 0)
		{
			//XMMATRIX currentMatrix{ pNode->Matrix };
			//model::Node* currentParent{ pNode->Parent };
			//while (currentParent)
			//{
			//	currentMatrix = currentParent->Matrix * currentMatrix;
			//	currentParent = currentParent->Parent;
			//}

			//m_ConstBuffer->Update(m_cbData);
			//pNode->Mesh->m_Matrix
			m_cbData.WVP = XMMatrixTranspose(pNode->Mesh->m_Matrix * m_WorldMatrix * pCamera->GetViewProjection());
			std::memcpy(m_ConstBuffer->pDataBegin, &m_cbData, sizeof(m_cbData));
			m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(0, m_ConstBuffer->GetBuffer()->GetGPUVirtualAddress());

			for (auto& primitive : pNode->Mesh->Primitives)
			{

				if (primitive->Material->BaseColorTexture != nullptr)
				{
					m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(1, primitive->Material->BaseColorTexture->m_Descriptor.m_gpuHandle);
				}

				if (primitive->Material->NormalTexture != nullptr)
				{
					m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(2, primitive->Material->NormalTexture->m_Descriptor.m_gpuHandle);
				}

				if (primitive->bHasIndices)
				{
					m_Device->GetCommandList()->IASetIndexBuffer(&m_IndexBuffer->GetBufferView());
					m_Device->GetCommandList()->DrawIndexedInstanced(primitive->IndexCount, 1,
																	 primitive->FirstIndexLocation,
																	 primitive->BaseVertexLocation, 0);
				}
				else
				{
					m_Device->GetCommandList()->DrawInstanced(primitive->VertexCount, 1,
															  primitive->StartVertexLocation, 0);
				}
			}
		}
		

		
	}

	for (auto& child : pNode->Children)
	{
		DrawNode(child, pCamera);
	}
}

void Model::Draw(Camera* pCamera)
{
	m_Device->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Device->GetCommandList()->IASetVertexBuffers(0, 1, &m_VertexBuffer->GetBufferView());

	for (auto& node : m_Importer->m_Nodes)
	{
		DrawNode(node, pCamera);
	}
}

void Model::DrawMeshes(Camera* pCamera)
{
	//m_Device->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//m_Device->GetCommandList()->IASetVertexBuffers(0, 1, &m_VertexBuffer->GetBufferView());
	auto currentFrameIndex{ m_Device->m_FrameIndex };
	for (auto mesh : m_Importer->m_Meshes)
	{
		UpdateWorld();

		//m_cbDatas.at(currentFrameIndex).WVP = XMMatrixTranspose(mesh->m_Matrix * m_WorldMatrix * pCamera->GetViewProjection());
		//m_cbDatas.at(currentFrameIndex).World = XMMatrixTranspose(m_WorldMatrix * XMMatrixIdentity());
		//std::memcpy(m_ConstBuffers.at(currentFrameIndex)->pDataBegin, &m_cbDatas.at(currentFrameIndex), sizeof(m_cbDatas.at(currentFrameIndex)));

		//m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(0, m_ConstBuffers.at(currentFrameIndex)->GetBuffer()->GetGPUVirtualAddress());

		mesh->Draw();
		/*
		if (mesh->Primitives.empty())
			return;

		for (auto& primitive : mesh->Primitives)
		{
			if (primitive->Material->BaseColorTexture)
				m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(1, primitive->Material->BaseColorTexture->m_Descriptor.m_gpuHandle);

			if (primitive->bHasIndices)
			{
				m_Device->GetCommandList()->IASetIndexBuffer(&m_IndexBuffer->GetBufferView());
				m_Device->GetCommandList()->DrawIndexedInstanced(primitive->IndexCount, 1,
																 primitive->FirstIndexLocation,
																 primitive->BaseVertexLocation, 0);
			}
			else
			{
				m_Device->GetCommandList()->DrawInstanced(primitive->VertexCount, 1,
														  primitive->StartVertexLocation, 0);
			}
		}
		*/
	}
}

void Model::Release()
{
	for (auto& node : m_Nodes)
	{
		for (auto& child : node->Children)
			delete child;
	}
}

void Model::DrawGUI()
{
	{
		ImGui::Begin("Model");

		if (ImGui::DragFloat3("Translation", m_Translations.data()))
		{
			m_Translation.m128_f32[0] = m_Translations.at(0);
			m_Translation.m128_f32[1] = m_Translations.at(1);
			m_Translation.m128_f32[2] = m_Translations.at(2);
		}

		if (ImGui::DragFloat4("Rotation", m_Rotations.data()))
		{
			m_Rotation.m128_f32[0] = XMConvertToRadians(m_Rotations.at(0));
			m_Rotation.m128_f32[1] = XMConvertToRadians(m_Rotations.at(1));
			m_Rotation.m128_f32[2] = XMConvertToRadians(m_Rotations.at(2));
			m_Rotation.m128_f32[3] = 0.0f;
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

void Model::UpdateWorld()
{
	m_WorldMatrix = XMMatrixIdentity();
	m_WorldMatrix = XMMatrixScalingFromVector(m_Scale) * XMMatrixRotationRollPitchYawFromVector(m_Rotation) * XMMatrixTranslationFromVector(m_Translation);
}
