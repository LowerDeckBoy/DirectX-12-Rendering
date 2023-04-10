#include "../../Core/Device.hpp"
#include "cglTF_Loader.hpp"
#include "../../Utils/FileUtils.hpp"
#include "../../Utils/TimeUtils.hpp"
#include <cassert>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

cglTF_Loader::cglTF_Loader(Device* pDevice, std::string_view Filepath) 	
{
}

bool cglTF_Loader::Import(Device* pDevice, std::string_view Filepath)
{
	if (files::GetExtension(Filepath.data()) != ".gltf")
	{
		throw std::exception();
	}

	assert(m_Device = pDevice);

	m_ModelPath = Filepath;

	TimeUtils timer{};
	timer.Timer_Start();

	cgltf_options options{};
	cgltf_data* data{};

	if (cgltf_parse_file(&options, Filepath.data(), &data) != cgltf_result_success)
	{
		::OutputDebugStringA("FAILED\n");
		throw std::exception();
	}
	
	if (cgltf_load_buffers(&options, data, Filepath.data()) != cgltf_result_success)
	{
		::OutputDebugStringA("FAILED\n");
		throw std::exception();
	}
	
	if (cgltf_validate(data) != cgltf_result_success)
	{
		::OutputDebugStringA("FAILED TO VALIDATE BUFFERS!\n");
		throw std::exception();
	}

	cgltf_scene* main_scene{ &data->scene[0] };
	
	for (size_t i = 0; i < main_scene->nodes_count; ++i)
	{
		ProcessNode(data, main_scene->nodes[i], nullptr, XMMatrixIdentity());
	}

	cgltf_free(data);
	timer.Timer_End(true);

	return true;
}

void cglTF_Loader::ProcessNode(cgltf_data* pData, cgltf_node* pNode, cglTF::Node* ParentNode, XMMATRIX ParentMatrix)
{
	cglTF::Node* newNode{ new cglTF::Node() };
	newNode->Name = pNode->name;
	newNode->Parent = ParentNode;

	if (pNode->has_matrix)
	{
		newNode->LocalMatrix = *(XMMATRIX*)pNode->matrix;
	}
	else
	{
		if (pNode->has_scale)
		{
			newNode->Scale = XMFLOAT3(pNode->scale[0], pNode->scale[1], pNode->scale[2]);
			newNode->LocalMatrix *= XMMatrixScalingFromVector(XMLoadFloat3(&newNode->Scale));
		}
		if (pNode->has_rotation)
		{
			newNode->Rotation = XMFLOAT4(pNode->rotation[0], pNode->rotation[1], pNode->rotation[2], pNode->rotation[3]);
			newNode->LocalMatrix *= XMMatrixRotationQuaternion(XMLoadFloat4(&newNode->Rotation));
		}
		if (pNode->has_translation)
		{
			newNode->Translation = XMFLOAT3(pNode->translation[0], pNode->translation[1], pNode->translation[2]);
			newNode->LocalMatrix *= XMMatrixTranslationFromVector(XMLoadFloat3(&newNode->Translation));
		}
	}

	newNode->Matrix = newNode->LocalMatrix * ParentMatrix;

	if (pNode->children)
	{
		for (size_t i = 0; i < pNode->children_count; ++i)
			ProcessNode(pData, pNode->children[i], newNode, newNode->Matrix);
	}
	
	if (pNode->mesh)
		newNode->Mesh = ProcessMesh(pData, pNode->mesh, newNode->Matrix);

	if (ParentNode)
		ParentNode->Children.emplace_back(newNode);
	else
		m_Nodes.emplace_back(newNode);

}

cglTF::Mesh* cglTF_Loader::ProcessMesh(cgltf_data* pData, cgltf_mesh* pMesh, XMMATRIX Matrix)
{
	cglTF::Mesh* newMesh{ new cglTF::Mesh() };
	newMesh->Matrix = Matrix;

	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT2> uvs;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT3> tangents;
	std::vector<XMFLOAT3> bitangents;
	std::vector<XMFLOAT4> colors;

	for (size_t i = 0; i < pMesh->primitives_count; ++i)
	{
		cglTF::Primitive* newPrimitive{ new cglTF::Primitive() };
		cgltf_primitive* primitive{ &pMesh->primitives[i] };

		newPrimitive->BaseVertexLocation  = static_cast<uint32_t>(m_Vertices.size());
		newPrimitive->FirstIndexLocation  = static_cast<uint32_t>(m_Indices.size());
		newPrimitive->StartVertexLocation = static_cast<uint32_t>(m_Vertices.size());

		for (size_t j = 0; j < primitive->attributes_count; ++j)
		{
			const cgltf_attribute* attribute{ &primitive->attributes[j] };
			
			const cgltf_accessor* accessor{ attribute->data };
			const cgltf_buffer_view* bufferView{ accessor->buffer_view };
			const cgltf_buffer* buffer{ bufferView->buffer };
			
			//const unsigned char* data = (unsigned char*)buffer->data + bufferView->offset + accessor->offset;
			const auto data = (const uint8_t*)buffer->data + bufferView->offset + accessor->offset;
			const size_t stride{ accessor->stride };

			const uint32_t vertexCount{ static_cast<uint32_t>(accessor->count) };
			newPrimitive->VertexCount = vertexCount;
			
			if (attribute->type == cgltf_attribute_type_position)
			{
				positions.reserve(vertexCount);
				for (size_t p = 0; p < vertexCount; ++p)
				{
					positions.push_back(*(XMFLOAT3*)((size_t)data + p * stride));
				}
			}
			else if (attribute->type == cgltf_attribute_type_texcoord)
			{
				uvs.reserve(vertexCount);
				if (attribute->data->component_type == cgltf_component_type_r_32f)
				{
					for (size_t uv = 0; uv < vertexCount; ++uv)
					{
						uvs.emplace_back(*(XMFLOAT2*)((size_t)data + uv * stride));
					}
				}
				else if (attribute->data->component_type == cgltf_component_type_r_16u)
				{
					for (size_t uv = 0; uv < vertexCount; ++uv)
					{
						uint16_t const& s = *(uint16_t*)((size_t)data + uv * stride + 0 * sizeof(uint16_t));
						uint16_t const& t = *(uint16_t*)((size_t)data + uv * stride + 1 * sizeof(uint16_t));
						//uvs.emplace_back(DirectX::XMFLOAT2(s / 65535.0f, 1.0f - t / 65535.0f));
						uvs.emplace_back(DirectX::XMFLOAT2(s, t));
					}
				}
				else if (attribute->data->component_type == cgltf_component_type_r_8u)
				{
					for (size_t uv = 0; uv < vertexCount; ++uv)
					{
						auto const& s = *(uint8_t*)((size_t)data + uv * stride + 0 * sizeof(uint8_t));
						auto const& t = *(uint8_t*)((size_t)data + uv * stride + 1 * sizeof(uint8_t));
						//uvs.emplace_back(DirectX::XMFLOAT2(s / 255.0f, 1.0f - t / 255.0f));
						uvs.emplace_back(DirectX::XMFLOAT2(s, t));
					}
				} 
				else
				{
					throw std::exception("Invalid UV type!");
				}
			}
			else if (attribute->type == cgltf_attribute_type_normal)
			{
				normals.reserve(vertexCount);
				for (size_t n = 0; n < vertexCount; ++n)
					normals.emplace_back(*(XMFLOAT3*)((size_t)data + n * stride));
			}
			else if (attribute->type == cgltf_attribute_type_tangent)
			{
				tangents.reserve(vertexCount);
				for (size_t t = 0; t < vertexCount; ++t)
					tangents.emplace_back(*(XMFLOAT3*)((size_t)data + t * stride));
			}
		}
		
		// Indices
		const bool hasIndices{ primitive->indices->count > 0 };
		newPrimitive->bHasIndices = hasIndices;
		if (hasIndices)
		{
			const cgltf_accessor* accessor{ primitive->indices };
			const cgltf_buffer_view* bufferView{ accessor->buffer_view };
			const cgltf_buffer* buffer{ bufferView->buffer };

			const unsigned char* data{ (unsigned char*)buffer->data + bufferView->offset + accessor->offset };
			const size_t stride{ accessor->stride };
			
			if (stride == 1)
			{
				for (size_t index = 0; index < primitive->indices->count; index += 3)
				{
					m_Indices.push_back(((uint8_t*)data)[index + 0]);
					m_Indices.push_back(((uint8_t*)data)[index + 1]);
					m_Indices.push_back(((uint8_t*)data)[index + 2]);
				}
			}
			else if (stride == 2)
			{
				for (size_t index = 0; index < primitive->indices->count; index += 3)
				{
					m_Indices.push_back(((uint16_t*)data)[index + 0]);
					m_Indices.push_back(((uint16_t*)data)[index + 1]);
					m_Indices.push_back(((uint16_t*)data)[index + 2]);
				}
			}
			else if (stride == 4)
			{
				for (size_t index = 0; index < primitive->indices->count; index += 3)
				{
					m_Indices.push_back(((uint32_t*)data)[index + 0]);
					m_Indices.push_back(((uint32_t*)data)[index + 1]);
					m_Indices.push_back(((uint32_t*)data)[index + 2]);
				}
			}
		}

		newPrimitive->Material = *ProcessMaterial(pData, primitive);
		newPrimitive->IndexCount = static_cast<uint32_t>(primitive->indices->count);
		newMesh->Primitives.emplace_back(newPrimitive);
	}

	size_t count = positions.size();
	if (uvs.size() != count)
		uvs.resize(count);
	if (normals.size() != count)
		normals.resize(count);
	if (tangents.size() != count)
		tangents.resize(count);

	for (size_t i = 0; i < count; ++i)
	{
		m_Vertices.push_back({  positions.at(i), 
								uvs.at(i),
								normals.at(i),
								XMFLOAT3(0.0f, 0.0f, 0.0f),
								XMFLOAT3(0.0f, 0.0f, 0.0f) });
	}
	
	return newMesh;
}

cglTF::Material* cglTF_Loader::ProcessMaterial(cgltf_data* pData, cgltf_primitive* pPrimitive)
{
	cglTF::Material* newMaterial{ new cglTF::Material() };

	//if (!pPrimitive->material)
	//	return newMaterial;

	cgltf_material* material = pPrimitive->material;
	cgltf_pbr_metallic_roughness* pbrMaterial = &material->pbr_metallic_roughness;

	newMaterial->AlphaCutoff = material->alpha_cutoff;
	/*
	if (material->normal_texture.texture)
	{
		auto uri{ material->normal_texture.texture->image->uri };
		auto texturePath{ files::glTF::GetTexturePath(m_ModelPath.data(), uri) };
		auto msg{ files::glTF::GetTexturePath(m_ModelPath.data(), uri) + '\n' };
		::OutputDebugStringA(msg.c_str());
		newMaterial->NormalTexture = new Texture(m_Device, texturePath);
	}

	if (material->emissive_texture.texture)
	{
		auto uri{ material->emissive_texture.texture->image->uri };
		auto texturePath{ files::glTF::GetTexturePath(m_ModelPath.data(), uri) };
		auto msg{ files::glTF::GetTexturePath(m_ModelPath.data(), uri) + '\n' };
		::OutputDebugStringA(msg.c_str());
		newMaterial->EmissiveFactor = *(XMFLOAT4*)material->emissive_factor;
		newMaterial->EmissiveTexture = new Texture(m_Device, texturePath);
	}
	*/
	if (!material->has_pbr_metallic_roughness)
		return newMaterial;
	

	if (pbrMaterial->base_color_texture.texture)
	{
		auto uri{ material->pbr_metallic_roughness.base_color_texture.texture->image->uri };
		auto texturePath{ files::glTF::GetTexturePath(m_ModelPath.data(), uri) };
		auto msg{ files::glTF::GetTexturePath(m_ModelPath.data(), uri) + '\n' };
		::OutputDebugStringA(msg.c_str());

		newMaterial->BaseColorFactor = *(XMFLOAT4*)(material->pbr_metallic_roughness.base_color_factor);
		newMaterial->BaseColorTexture = new Texture(m_Device, texturePath);
	}
	/*
	if (pbrMaterial->metallic_roughness_texture.texture)
	{
		auto uri{ pbrMaterial->metallic_roughness_texture.texture->image->uri };
		auto texturePath{ files::glTF::GetTexturePath(m_ModelPath.data(), uri) };
		auto msg{ files::glTF::GetTexturePath(m_ModelPath.data(), uri) + '\n' };
		::OutputDebugStringA(msg.c_str());

		newMaterial->MetallicFactor = pbrMaterial->metallic_factor;
		newMaterial->RoughnessFactor = pbrMaterial->roughness_factor;
		newMaterial->MetallicTexture = new Texture(m_Device, texturePath);
	}
	*/
	

	return newMaterial;
}
