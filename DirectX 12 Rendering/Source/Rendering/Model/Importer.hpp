#pragma once
#include "Mesh.hpp"

class DeviceContext;
struct aiScene;
struct aiNode;
struct aiMesh;

class Importer
{
public:
	Importer() = default;
	Importer(DeviceContext* pDevice, std::string_view Filepath);
	virtual ~Importer() {};

	bool Import(DeviceContext* pDevice, std::string_view Filepath);

	void ProcessNode(const aiScene* pScene, const aiNode* pNode, model::Node* ParentNode, XMMATRIX ParentMatrix);
	model::Mesh* ProcessMesh(const aiScene* pScene, const aiMesh* pMesh, XMMATRIX Matrix);
	void ProcessMaterials(const aiScene* pScene, const aiMesh* pMesh);

	void ProcessAnimations(const aiScene* pScene);

protected:
	DeviceContext* m_Device{ nullptr };
	std::string_view m_ModelPath;

	std::string m_ModelName;

	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	//std::vector<model::Node*> m_Nodes;
	std::vector<model::Mesh*> m_Meshes;

	//std::vector<model::Material*> m_Materials;
	std::vector<model::MaterialData*> m_Materials;
	std::vector<Texture*> m_Textures;

	bool bHasAnimations{ false };

};

