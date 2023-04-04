#pragma once

#ifndef STBI_MSC_SECURE_CRT
#define STBI_MSC_SECURE_CRT
#endif

#include "../Graphics/Vertex.hpp"
#include "../Graphics/Buffer.hpp"
#include "../Graphics/ConstantBuffer.hpp"

#include "Material.hpp"
#include "Primitive.hpp"
#include "Mesh.hpp"
#include "Node.hpp"

#include <tiny_gltf.h>


class Device;

class ModelImporter
{
public:
	ModelImporter() { }

	explicit ModelImporter(Device* pDevice) : m_Device(pDevice) {}
	~ModelImporter() {}

	std::string m_FilePath;

	std::vector<model::Node*> m_Nodes;
	std::vector<model::Node*> m_LinearNodes;

	bool ImportModel(Device* pDevice, const std::string& PathToModel, std::vector<Mesh*>& Meshes);
	void TraverseNode(const tinygltf::Model& Model, model::Node* ParentNode, const tinygltf::Node& Node, uint32_t CurrentIndex, XMMATRIX ParentMatrix);
	Mesh* ProcessMesh(const tinygltf::Model& Model, const tinygltf::Mesh& inMesh, XMMATRIX& Matrix);

	void GatherMaterials(const tinygltf::Model& Model, Device* pDevice);

	void GatherMeshes(const tinygltf::Model& Model, const tinygltf::Mesh& inMesh, XMMATRIX Matrix);
	void ProcessNodes(const tinygltf::Model& Model, int32_t NodeIndex, XMMATRIX Matrix = XMMatrixIdentity());
		
	// Total counts for single buffer usage
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	//TEST
	std::vector<Mesh*> m_Meshes;

private:
	Device* m_Device;
};

