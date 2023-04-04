#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define TINYGLTF_USE_CPP14
#include "../Core/Device.hpp"
#include "ModelImporter.hpp"
#include "../Utils/Utils.hpp"
#include "../Utils/FileHelper.hpp"

bool ModelImporter::ImportModel(Device* pDevice, const std::string& PathToModel, std::vector<Mesh*>& Meshes)
{
	// Check extension here
	m_Device = pDevice;

	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	std::string warning;
	std::string error;

	bool result = loader.LoadASCIIFromFile(&model, &error, &warning, PathToModel);

	if (!warning.empty())
	{
		auto msg{ "[glTF model] Warning: " + warning + "!\n" };
		::OutputDebugStringA(msg.c_str());
	}

	if (!error.empty())
	{
		auto msg{ "[glTF model] Error: " + error + "!\n" };
		::OutputDebugStringA(msg.c_str());
		return false;
	}

	if (!result)
	{
		auto msg{ "[glTF model] Failed to load!\n" };
		::OutputDebugStringA(msg);
		return false;
	}

	m_FilePath = PathToModel;

	const tinygltf::Scene& scene = model.scenes[0];
	//for (size_t i = 0; i < scene.nodes.size(); i++)
	//{
	//	const tinygltf::Node currentNode{ model.nodes[scene.nodes.at(i)] };
	//	TraverseNode(model, nullptr, currentNode, static_cast<uint32_t>(i), XMMatrixIdentity());
	//}
	TraverseNode(model, nullptr, model.nodes[scene.nodes.at(0)], 0, XMMatrixIdentity());
	//TraverseNodes(model, 0);

	//for (const auto& mesh : model.meshes)
	//{
	//	//Mesh* mesh = ProcessMesh(model, mesh, XMMatrixIdentity());
	//	GatherMeshes(model, mesh);
	//}

	//ProcessNodes(model, 0);

	if (!model.materials.empty())
	{
		//GatherMaterials(model);
	}

	//for (auto& node : m_Nodes) {
	//	node->Matrix = node->GetLocalMatrix();
	//}
	//for (auto& mesh : m_Meshes)
	//	Meshes.emplace_back(mesh);

	return true;
}

void ModelImporter::TraverseNode(const tinygltf::Model& Model, model::Node* ParentNode, const tinygltf::Node& Node, uint32_t CurrentIndex, XMMATRIX ParentMatrix)
{
	model::Node* newNode{ new model::Node() };
	newNode->Parent = ParentNode;
	newNode->Index = CurrentIndex;
	newNode->Name = Node.name;

	if (!Node.matrix.empty())
	{
		XMMATRIX matrix{ XMMatrixIdentity() };
		XMFLOAT4X4 mat{};
		mat._11 = static_cast<float>(Node.matrix.at(0));
		mat._12 = static_cast<float>(Node.matrix.at(1));
		mat._13 = static_cast<float>(Node.matrix.at(2));
		mat._14 = static_cast<float>(Node.matrix.at(3));
		mat._21 = static_cast<float>(Node.matrix.at(4));
		mat._22 = static_cast<float>(Node.matrix.at(5));
		mat._23 = static_cast<float>(Node.matrix.at(6));
		mat._24 = static_cast<float>(Node.matrix.at(7));
		mat._31 = static_cast<float>(Node.matrix.at(8));
		mat._32 = static_cast<float>(Node.matrix.at(9));
		mat._33 = static_cast<float>(Node.matrix.at(10));
		mat._34 = static_cast<float>(Node.matrix.at(11));
		mat._41 = static_cast<float>(Node.matrix.at(12));
		mat._42 = static_cast<float>(Node.matrix.at(13));
		mat._43 = static_cast<float>(Node.matrix.at(14));
		mat._44 = static_cast<float>(Node.matrix.at(15));
		matrix = XMLoadFloat4x4(&mat);
		newNode->Matrix = matrix;
		//newNode->LocalMatrix = matrix;
	}
	else
	{
		XMFLOAT3 translation{ XMFLOAT3(0.0f, 0.0f, 0.0f) };
		XMFLOAT4 rotation{ XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f) };
		XMFLOAT3 scale{ XMFLOAT3(1.0f, 1.0f, 1.0f) };
		if (!Node.translation.empty())
		{
			translation = XMFLOAT3(Node.translation.at(0), Node.translation.at(1), Node.translation.at(2));
			newNode->Translation = translation;
		}
		if (!Node.rotation.empty())
		{
			rotation = XMFLOAT4(Node.rotation.at(0), Node.rotation.at(1), Node.rotation.at(2), Node.rotation.at(3));
			newNode->Rotation = rotation;
		}
		if (!Node.scale.empty())
		{
			scale = XMFLOAT3(Node.scale.at(0), Node.scale.at(1), Node.scale.at(2));
			newNode->Scale = scale;
		}
	}
	// * 
	XMMATRIX local{ XMMatrixScalingFromVector(XMLoadFloat3(&newNode->Scale)) *
					XMMatrixRotationQuaternion(XMLoadFloat4(&newNode->Rotation)) * 
					XMMatrixTranslationFromVector(XMLoadFloat3(&newNode->Translation)) };

	//XMMATRIX local{ XMMatrixTranslationFromVector(XMLoadFloat3(&newNode->Translation))  * XMMatrixRotationQuaternion(XMLoadFloat4(&newNode->Rotation)) * XMMatrixScalingFromVector(XMLoadFloat3(&newNode->Scale)) };

	//XMMATRIX next{ local * ParentMatrix };
	XMMATRIX next{ local };
	
	if (!Node.children.empty())
	{
		for (auto& child : Node.children)
			TraverseNode(Model, newNode, Model.nodes[child], child, next);
	}

	if (Node.mesh >= 0)
	{
		newNode->Mesh = ProcessMesh(Model, Model.meshes[Node.mesh], next);
		//m_Meshes.emplace_back(newNode->Mesh);
	}

	if (ParentNode)
		ParentNode->Children.emplace_back(newNode);
	else
		m_Nodes.emplace_back(newNode);

	//m_LinearNodes.emplace_back(newNode);

}

Mesh* ModelImporter::ProcessMesh(const tinygltf::Model& Model, const tinygltf::Mesh& inMesh, XMMATRIX& Matrix)
{
	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT2> uvs;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT3> tangents;
	std::vector<XMFLOAT3> bitangents;
	std::vector<float> tangentWS;

	Mesh* newMesh{ new Mesh() };
	newMesh->m_Matrix = Matrix;
	for (const auto& primitive : inMesh.primitives)
	{
		bool bHasIndices{ primitive.indices >= 0 };

		const auto& indexAccessor{ Model.accessors[primitive.indices] };

		Primitive* newPrimitive{ new Primitive() };
		newPrimitive->BaseVertexLocation = static_cast<uint32_t>(m_Vertices.size());
		newPrimitive->FirstIndexLocation = static_cast<uint32_t>(m_Indices.size());
		newPrimitive->IndexCount = static_cast<uint32_t>(indexAccessor.count);
		newPrimitive->bHasIndices = bHasIndices;
		// Vertices begin
		for (const auto& attribute : primitive.attributes)
		{
			const std::string attribName{ attribute.first };
			const int attribData{ attribute.second };

			const auto& vertexAccessor{ Model.accessors[attribData] };
			const auto& bufferView{ Model.bufferViews[vertexAccessor.bufferView] };
			const auto& buffer{ Model.buffers[bufferView.buffer] };

			const unsigned char* data{ buffer.data.data() + bufferView.byteOffset + vertexAccessor.byteOffset };
			const int32_t stride{ vertexAccessor.ByteStride(bufferView) };

			const uint32_t vertexCount = static_cast<uint32_t>(vertexAccessor.count);
			newPrimitive->VertexCount = vertexCount;
			newPrimitive->StartVertexLocation = static_cast<uint32_t>(m_Vertices.size());

			if (attribName == "POSITION")
			{
				positions.reserve(vertexCount);
				for (size_t i = 0; i < vertexCount; ++i)
					positions.emplace_back(*(XMFLOAT3*)((size_t)data + i * stride));
			}
			else if (attribName == "TEXCOORD_0")
			{
				uvs.reserve(vertexCount);
				if (vertexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
				{
					for (size_t i = 0; i < vertexCount; ++i)
					{
						XMFLOAT2 uv{ *(XMFLOAT2*)((size_t)data + i * stride) };
						uvs.emplace_back(uv);
					}
				}
				/*
				else if (vertexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
				{
					for (size_t i = 0; i < vertexCount; ++i)
					{
						auto const& s = *(UINT8*)((size_t)data + i * stride + 0 * sizeof(UINT8));
						auto const& t = *(UINT8*)((size_t)data + i * stride + 1 * sizeof(UINT8));
						uvs.push_back(DirectX::XMFLOAT2(s / 255.0f, 1.0f - t / 255.0f));
					}
				}
				else if (vertexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
				{
					for (size_t i = 0; i < vertexCount; ++i)
					{
						uint16_t const& s = *(UINT16*)((size_t)data + i * stride + 0 * sizeof(UINT16));
						uint16_t const& t = *(UINT16*)((size_t)data + i * stride + 1 * sizeof(UINT16));
						uvs.emplace_back(DirectX::XMFLOAT2(s / 65535.0f, 1.0f - t / 65535.0f));
					}
				}*/
			}
			else if (attribName == "NORMAL")
			{
				normals.reserve(vertexCount);
				for (size_t i = 0; i < vertexCount; ++i)
					normals.emplace_back(*(XMFLOAT3*)((size_t)data + i * stride));
			}
			else if (attribName == "TANGENT")
			{
				tangents.reserve(vertexCount);
				for (size_t i = 0; i < vertexCount; ++i)
				{
					DirectX::XMFLOAT4 tangent = *(DirectX::XMFLOAT4*)((size_t)data + i * stride);
					tangents.emplace_back(tangent.x, tangent.y, tangent.z);

					tangentWS.push_back(tangent.w);
				}
			}
		}

		size_t count{ positions.size() };
		if (uvs.size() != count) uvs.resize(count);
		if (normals.size() != count) normals.resize(count);
		if (tangents.size() != count) tangents.resize(count);
		if (bitangents.size() != count) bitangents.resize(count);
		if (tangentWS.size() != count) tangentWS.resize(count);

		//vertices.reserve(vertices.size() + count);
		for (size_t i = 0; i < count; ++i)
		{
			XMVECTOR _bitangent = XMVectorScale(XMVector3Cross(XMLoadFloat3(&normals.at(i)),
												XMLoadFloat3(&tangents.at(i))),
												tangentWS.at(i));
			XMStoreFloat3(&bitangents.at(i), XMVector3Normalize(_bitangent));

			m_Vertices.push_back({ positions.at(i), uvs.at(i), normals.at(i), tangents.at(i), bitangents.at(i) });
		}
		// Vertices end

		// Indices begin
		if (bHasIndices)
		{
			const auto& indexBufferView{ Model.bufferViews[indexAccessor.bufferView] };
			const auto& indexBuffer{ Model.buffers[indexBufferView.buffer] };

			const int32_t stride{ indexAccessor.ByteStride(indexBufferView) };

			const uint32_t indexCount = static_cast<uint32_t>(indexAccessor.count);

			const unsigned char* data{ indexBuffer.data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset };

			if (stride == 1)
			{
				for (size_t i = 0; i < indexCount; i += 3)
				{
					m_Indices.push_back(data[i + 0]);
					m_Indices.push_back(data[i + 1]);
					m_Indices.push_back(data[i + 2]);
				}
			}
			else if (stride == 2)
			{
				for (size_t i = 0; i < indexCount; i += 3)
				{
					m_Indices.push_back(((uint16_t*)data)[i + 0]);
					m_Indices.push_back(((uint16_t*)data)[i + 1]);
					m_Indices.push_back(((uint16_t*)data)[i + 2]);
				}
			}
			else if (stride == 4)
			{
				for (size_t i = 0; i < indexCount; i += 3)
				{
					m_Indices.push_back(((uint32_t*)data)[i + 0]);
					m_Indices.push_back(((uint32_t*)data)[i + 1]);
					m_Indices.push_back(((uint32_t*)data)[i + 2]);
				}
			}
		// Indices end
		}


		Material* newMaterial{ new Material() };
		// Materials begin
		{
			const tinygltf::Material gltfMaterial{ Model.materials[primitive.material] };
			const tinygltf::PbrMetallicRoughness pbrMaterial{ gltfMaterial.pbrMetallicRoughness };

			//;
			//newPrimitive->Material->BaseColor = (DirectX::XMFLOAT4)pbrMaterial.baseColorFactor.data();
			//newPrimitive->Material->BaseColor.x = static_cast<float>(pbrMaterial.baseColorFactor[0]);
			//newPrimitive->Material->BaseColor.y = static_cast<float>(pbrMaterial.baseColorFactor[1]);
			//newPrimitive->Material->BaseColor.z = static_cast<float>(pbrMaterial.baseColorFactor[2]);
			//newPrimitive->Material->BaseColor.w = static_cast<float>(pbrMaterial.baseColorFactor[3]);

			if (pbrMaterial.baseColorTexture.index >= 0)
			{
				const auto& texture{ Model.textures[pbrMaterial.baseColorTexture.index] };
				const auto& image{ Model.images[texture.source] };

				const auto& imageUri{ files::glTF::GetTexturePath(m_FilePath, image.uri) };
				newMaterial->BaseColorTexture = new Texture(m_Device, imageUri);

			}
			if (gltfMaterial.normalTexture.index >= 0)
			{
				const auto& texture{ Model.textures[gltfMaterial.normalTexture.index] };
				const auto& image{ Model.images[texture.source] };
				
				const auto& imageUri{ files::glTF::GetTexturePath(m_FilePath, image.uri) };
				newMaterial->NormalTexture = new Texture(m_Device, imageUri);
			}

		}
		newPrimitive->Material = newMaterial;
		// Materials end

		newMesh->Primitives.push_back(newPrimitive);
		// test
		//m_Meshes.emplace_back(newMesh);
	}

	return newMesh;
}

void ModelImporter::GatherMaterials(const tinygltf::Model& Model, Device* pDevice)
{

	for (const auto& material : Model.materials)
	{
		Material newMaterial{};

		const auto& pbrMaterial{ material.pbrMetallicRoughness };

		//if (pbrMaterial.baseColorFactor.size() == 4)
		//{
		//	newMaterial.BaseColor
		//}

		if (pbrMaterial.baseColorTexture.index >= 0)
		{
			const auto& texture{ Model.textures[pbrMaterial.baseColorTexture.index] };
			const auto& image{ Model.images[texture.source] };

			auto imageUri{ files::glTF::GetTexturePath(m_FilePath, image.uri) };
			auto msg{ "Texture path: " + imageUri + '\n' };
			::OutputDebugStringA(msg.c_str());

			//newMaterial.BaseColor = *(XMFLOAT4*)pbrMaterial.baseColorFactor.data();
			newMaterial.BaseColorTexture = new Texture(pDevice, imageUri);
			newMaterial.BaseColorTexture->Create(pDevice, imageUri);
			newMaterial.BaseColorTexture->GetTexture()->SetName(L"Base Color Tex");
		}
	}
}

void ModelImporter::GatherMeshes(const tinygltf::Model& Model, const tinygltf::Mesh& inMesh, XMMATRIX Matrix)
{
	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT2> uvs;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT3> tangents;
	std::vector<XMFLOAT3> bitangents;
	std::vector<float> tangentWS;

	for (const auto& primitive : inMesh.primitives)
	{
		Mesh* newMesh{ new Mesh(m_Device) };
		Primitive* newPrimitive{ new Primitive() };
		newPrimitive->bHasIndices = (primitive.indices >= 0);
		newPrimitive->BaseVertexLocation = static_cast<uint32_t>(m_Vertices.size());
		newPrimitive->FirstIndexLocation = static_cast<uint32_t>(m_Indices.size());

		const auto& indexAccessor{ Model.accessors[primitive.indices] };
		newPrimitive->IndexCount = static_cast<uint32_t>(indexAccessor.count);

		for (const auto& attribute : primitive.attributes)
		{
			const std::string attribName{ attribute.first };
			const int attribData{ attribute.second };

			const auto& vertexAccessor{ Model.accessors[attribData] };
			const auto& bufferView{ Model.bufferViews[vertexAccessor.bufferView] };
			const auto& buffer{ Model.buffers[bufferView.buffer] };

			const int32_t stride{ vertexAccessor.ByteStride(bufferView) };
			const unsigned char* data{ buffer.data.data() + bufferView.byteOffset + vertexAccessor.byteOffset };

			const uint32_t vertexCount = static_cast<uint32_t>(vertexAccessor.count);
			newPrimitive->VertexCount = vertexCount;
			newPrimitive->StartVertexLocation = static_cast<uint32_t>(m_Vertices.size());

			if (attribName == "POSITION")
			{
				positions.reserve(vertexCount);
				for (size_t i = 0; i < vertexCount; ++i)
					positions.emplace_back(*(XMFLOAT3*)(data + i * stride));
			}
			else if (attribName == "TEXCOORD_0")
			{
				uvs.reserve(vertexCount);
				if (vertexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
				{
					for (size_t i = 0; i < vertexCount; ++i)
					{
						XMFLOAT2 uv{ *(XMFLOAT2*)((size_t)data + i * stride) };
						uvs.emplace_back(uv);
					}
				}
				else if (vertexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
				{
					for (size_t i = 0; i < vertexCount; ++i)
					{
						auto const& s = *(UINT8*)((size_t)data + i * stride + 0 * sizeof(UINT8));
						auto const& t = *(UINT8*)((size_t)data + i * stride + 1 * sizeof(UINT8));
						uvs.push_back(DirectX::XMFLOAT2(s / 255.0f, 1.0f - t / 255.0f));
					}
				}
				else if (vertexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
				{
					for (size_t i = 0; i < vertexCount; ++i)
					{
						uint16_t const& s = *(UINT16*)((size_t)data + i * stride + 0 * sizeof(UINT16));
						uint16_t const& t = *(UINT16*)((size_t)data + i * stride + 1 * sizeof(UINT16));
						uvs.emplace_back(DirectX::XMFLOAT2(s / 65535.0f, 1.0f - t / 65535.0f));
					}
				}
			}
			else if (attribName == "NORMAL")
			{
				normals.reserve(vertexCount);
				for (size_t i = 0; i < vertexCount; ++i)
					normals.emplace_back(*(XMFLOAT3*)(data + i * stride));
			}
			else if (attribName == "TANGENT")
			{
				tangents.reserve(vertexCount);
				for (size_t i = 0; i < vertexCount; ++i)
				{
					DirectX::XMFLOAT4 tangent = *(DirectX::XMFLOAT4*)((size_t)data + i * stride);
					tangents.emplace_back(tangent.x, tangent.y, tangent.z);

					tangentWS.push_back(tangent.w);
				}
			}

		}

		size_t count{ positions.size() };
		if (uvs.size() != count) uvs.resize(count);
		if (normals.size() != count) normals.resize(count);
		if (tangents.size() != count) tangents.resize(count);
		if (bitangents.size() != count) bitangents.resize(count);
		if (tangentWS.size() != count) tangentWS.resize(count);

		for (size_t i = 0; i < count; ++i)
		{
			XMVECTOR _bitangent = XMVectorScale(XMVector3Cross(XMLoadFloat3(&normals.at(i)),
												XMLoadFloat3(&tangents.at(i))),
												tangentWS.at(i));
			XMStoreFloat3(&bitangents.at(i), XMVector3Normalize(_bitangent));

			m_Vertices.push_back({ positions.at(i), uvs.at(i), normals.at(i), tangents.at(i), bitangents.at(i) });
		}
		
		if (primitive.indices >= 0)
		{
			const auto& bufferView{ Model.bufferViews[indexAccessor.bufferView] };
			const auto& buffer{ Model.buffers[bufferView.buffer] };

			const int32_t stride{ indexAccessor.ByteStride(bufferView) };

			const uint32_t indexCount = static_cast<uint32_t>(indexAccessor.count);

			const unsigned char* data{ buffer.data.data() + bufferView.byteOffset + indexAccessor.byteOffset };

			if (stride == 1)
			{
				for (size_t i = 0; i < indexCount; i += 3)
				{
					m_Indices.push_back(data[i + 0]);
					m_Indices.push_back(data[i + 1]);
					m_Indices.push_back(data[i + 2]);
				}
			}
			else if (stride == 2)
			{
				for (size_t i = 0; i < indexCount; i += 3)
				{
					m_Indices.push_back(((uint16_t*)data)[i + 0]);
					m_Indices.push_back(((uint16_t*)data)[i + 1]);
					m_Indices.push_back(((uint16_t*)data)[i + 2]);
				}
			}
			else if (stride == 4)
			{
				for (size_t i = 0; i < indexCount; i += 3)
				{
					m_Indices.push_back(((uint32_t*)data)[i + 0]);
					m_Indices.push_back(((uint32_t*)data)[i + 1]);
					m_Indices.push_back(((uint32_t*)data)[i + 2]);
				}
			}

			// Indices end
		}

		Material* newMaterial{ new Material() };
		// Materials begin
		{
			const tinygltf::Material gltfMaterial{ Model.materials[primitive.material] };
			const tinygltf::PbrMetallicRoughness pbrMaterial{ gltfMaterial.pbrMetallicRoughness };

			//;
			//newPrimitive->Material->BaseColor = (DirectX::XMFLOAT4)pbrMaterial.baseColorFactor.data();
			//newPrimitive->Material->BaseColor.x = static_cast<float>(pbrMaterial.baseColorFactor[0]);
			//newPrimitive->Material->BaseColor.y = static_cast<float>(pbrMaterial.baseColorFactor[1]);
			//newPrimitive->Material->BaseColor.z = static_cast<float>(pbrMaterial.baseColorFactor[2]);
			//newPrimitive->Material->BaseColor.w = static_cast<float>(pbrMaterial.baseColorFactor[3]);

			if (pbrMaterial.baseColorTexture.index >= 0)
			{
				const auto& texture{ Model.textures[pbrMaterial.baseColorTexture.index] };
				const auto& image{ Model.images[texture.source] };

				const auto& imageUri{ files::glTF::GetTexturePath(m_FilePath, image.uri) };
				newMaterial->BaseColorTexture = new Texture();
				newMaterial->BaseColorTexture->Create(m_Device, imageUri);

			}

			if (gltfMaterial.normalTexture.index >= 0)
			{
				//const auto& texture{ Model.textures[gltfMaterial.normalTexture.index] };
				//const auto& image{ Model.images[texture.source] };
				//
				//const auto& imageUri{ files::glTF::GetTexturePath(m_FilePath, image.uri) };
				//newMaterial->NormalTexture = new Texture();
				//newMaterial->NormalTexture->Create(m_Device, imageUri);
			}

			newPrimitive->Material = newMaterial;
		}
		// Materials end
		newMesh->Primitives.emplace_back(newPrimitive);

		newMesh->m_Matrix = Matrix;
	
		newMesh->SetVertices(m_Device, m_Vertices);
		newMesh->SetIndices(m_Device, m_Indices);
		m_Vertices.clear();
		m_Vertices.shrink_to_fit();
		m_Indices.clear();
		m_Indices.shrink_to_fit();

		m_Meshes.emplace_back(newMesh);
	}

}

void ModelImporter::ProcessNodes(const tinygltf::Model& Model, int32_t NodeIndex, XMMATRIX Matrix)
{
	const tinygltf::Node& currentNode{ Model.nodes[NodeIndex] };

	XMFLOAT3 translationTemp{ XMFLOAT3(0.0f, 0.0f, 0.0f) };
	if (!currentNode.translation.empty())
	{
		translationTemp.x = static_cast<float>(currentNode.translation[0]);
		translationTemp.y = static_cast<float>(currentNode.translation[1]);
		translationTemp.z = static_cast<float>(currentNode.translation[2]);
	}

	XMFLOAT4 rotationTemp{ XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	if (!currentNode.rotation.empty())
	{
		rotationTemp.x = static_cast<float>(currentNode.rotation[0]);
		rotationTemp.y = static_cast<float>(currentNode.rotation[1]);
		rotationTemp.z = static_cast<float>(currentNode.rotation[2]);
		rotationTemp.w = static_cast<float>(currentNode.rotation[3]);
	}

	XMFLOAT3 scaleTemp{ XMFLOAT3(1.0f, 1.0f, 1.0f) };
	if (!currentNode.scale.empty())
	{
		rotationTemp.x = static_cast<float>(currentNode.scale[0]);
		rotationTemp.y = static_cast<float>(currentNode.scale[1]);
		rotationTemp.z = static_cast<float>(currentNode.scale[2]);
	}

	XMMATRIX matrix{ XMMatrixIdentity() };
	if (!currentNode.matrix.empty())
	{
		XMFLOAT4X4 mat{};
		mat._11 = static_cast<float>(currentNode.matrix[0]);
		mat._12 = static_cast<float>(currentNode.matrix[1]);
		mat._13 = static_cast<float>(currentNode.matrix[2]);
		mat._14 = static_cast<float>(currentNode.matrix[3]);
		mat._21 = static_cast<float>(currentNode.matrix[4]);
		mat._22 = static_cast<float>(currentNode.matrix[5]);
		mat._23 = static_cast<float>(currentNode.matrix[6]);
		mat._24 = static_cast<float>(currentNode.matrix[7]);
		mat._31 = static_cast<float>(currentNode.matrix[8]);
		mat._32 = static_cast<float>(currentNode.matrix[9]);
		mat._33 = static_cast<float>(currentNode.matrix[10]);
		mat._34 = static_cast<float>(currentNode.matrix[11]);
		mat._41 = static_cast<float>(currentNode.matrix[12]);
		mat._42 = static_cast<float>(currentNode.matrix[13]);
		mat._43 = static_cast<float>(currentNode.matrix[14]);
		mat._44 = static_cast<float>(currentNode.matrix[15]);
		matrix = XMLoadFloat4x4(&mat);
	}

	XMVECTOR translation = XMLoadFloat3(&translationTemp);
	XMVECTOR rotation = XMLoadFloat4(&rotationTemp);
	XMVECTOR scale = XMLoadFloat3(&scaleTemp);
	//
	XMMATRIX world = matrix * XMMatrixScalingFromVector(scale) * XMMatrixRotationQuaternion(rotation) * XMMatrixTranslationFromVector(translation);
	
	XMMATRIX nextMatrix = Matrix * world;
	// 
	if (!currentNode.children.empty())
	{
		for (const auto& child : currentNode.children)
		{
			ProcessNodes(Model, child, nextMatrix);
		}
	}

	if (currentNode.mesh >= 0)
	{
		GatherMeshes(Model, Model.meshes[currentNode.mesh], nextMatrix);
	}


}
