#pragma once
#include "Mesh.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>
#include <assimp/GltfMaterial.h>

class Importer
{
public:
	Importer() = default;
	Importer(Device* pDevice, std::string_view Filepath);
	virtual ~Importer() {};

	bool Import(Device* pDevice, std::string_view Filepath);

	void ProcessNode(const aiScene* pScene, const aiNode* pNode, model::Node* ParentNode, XMMATRIX ParentMatrix);
	model::Mesh* ProcessMesh(const aiScene* pScene, const aiMesh* pMesh, XMMATRIX Matrix);
	void ProcessMaterials(const aiScene* pScene, const aiMesh* pMesh);

protected:
	Device* m_Device{ nullptr };
	std::string_view m_ModelPath;

	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	std::vector<model::Node*> m_Nodes;
	std::vector<model::Mesh*> m_Meshes;

	std::vector<model::Material*> m_Materials;


};

