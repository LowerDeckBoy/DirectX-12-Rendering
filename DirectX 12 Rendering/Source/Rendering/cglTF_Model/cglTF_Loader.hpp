#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/GltfMaterial.h>

#include "cglTF_Mesh.hpp"
//#include "../../Graphics/Vertex.hpp"

class Device;
struct cgltf_data;
struct cgltf_node;
struct cgltf_mesh;
struct cgltf_primitive;

class cglTF_Loader
{
public:
	cglTF_Loader() = default;
	cglTF_Loader(Device* pDevice, std::string_view Filepath);

	bool Import(Device* pDevice, std::string_view Filepath);

	// cgltf approach
	void ProcessNode(cgltf_data* pData, cgltf_node* pNode, cglTF::Node* ParentNode, XMMATRIX ParentMatrix);
	cglTF::Mesh* ProcessMesh(cgltf_data* pData, cgltf_mesh* pMesh, XMMATRIX Matrix);
	cglTF::Material* ProcessMaterial(cgltf_data* pData, cgltf_primitive* pPrimitive);

protected:
	Device* m_Device{ nullptr };
	std::string_view m_ModelPath;
	
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	std::vector<cglTF::Node*> m_Nodes;

};