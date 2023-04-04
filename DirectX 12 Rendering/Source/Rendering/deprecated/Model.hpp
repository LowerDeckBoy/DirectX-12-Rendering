#pragma once

#include "../Core/Device.hpp"
#include <vector>
#include "../Graphics/Vertex.hpp"
#include "../Graphics/Buffer.hpp"
#include "../Graphics/ConstantBuffer.hpp"

#include "Material.hpp"
#include "Mesh.hpp"
#include "Node.hpp"
#include "ModelImporter.hpp"

//class ModelImporter;

class Device;
class Camera;

class Model
{
public:
	~Model();

	void Initialize(Device* pDevice, const std::string& PathToModel);

	void Update();
	void DrawNode(model::Node* pNode, Camera* pCamera);
	void Draw(Camera* pCamera);

	void DrawMeshes(Camera* pCamera);

	void Release();

	void DrawGUI();

private:
	Device* m_Device{ nullptr };
	std::string m_ModelName{ "None Given" };
	std::string m_FilePath;

	std::unique_ptr<VertexBuffer<Vertex>> m_VertexBuffer{ nullptr };
	std::unique_ptr<IndexBuffer> m_IndexBuffer{ nullptr };

	std::unique_ptr<ConstantBuffer<cbPerObject>> m_ConstBuffer;
	cbPerObject m_cbData{};
	//std::array<std::unique_ptr<ConstantBuffer<cbPerObject>>, 2> m_ConstBuffers;
	// cBuffer data
	//std::array<cbPerObject, 2> m_cbDatas{};

	//XMMATRIX m_WorldMatrix{ XMMatrixIdentity() };

	std::vector<model::Node*> m_Nodes;
	//std::vector<model::Node*> m_LinearNodes;

	std::vector<Mesh*> m_Meshes;
	//std::vector<Material> m_Materials;

	// Model importing
	std::unique_ptr<ModelImporter> m_Importer;

	void UpdateWorld();
	XMMATRIX m_WorldMatrix{ XMMatrixIdentity() };
	XMVECTOR m_Translation{ XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
	XMVECTOR m_Rotation{ XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f) };
	XMVECTOR m_Scale{ XMVectorSet(1.0f, 1.0f, 1.0f, 0.0) };

	// For GUI usage
	std::array<float, 3> m_Translations{ 0.0f, 0.0f, 0.0f };
	std::array<float, 3> m_Rotations{ 0.0f, 0.0f, 0.0f };
	std::array<float, 3> m_Scales{ 1.0f, 1.0f, 1.0f };
};

