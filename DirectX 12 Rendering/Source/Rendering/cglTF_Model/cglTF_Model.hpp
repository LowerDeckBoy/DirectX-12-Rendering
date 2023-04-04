#pragma once
#include "cglTF_Loader.hpp"
#include "../../Graphics/Buffer.hpp"
#include "../../Graphics/ConstantBuffer.hpp"

#include <memory>

class Camera;
class Device;

class cglTF_Model : public cglTF_Loader
{
public:
	cglTF_Model() = default;
	cglTF_Model(Device* pDevice, std::string_view Filepath);

	void Create(Device* pDevice, std::string_view Filepath);

	void DrawNode(uint32_t CurrentFrame, Camera* pCamera, cglTF::Node* pNode);
	void Draw(uint32_t CurrentFrame, Camera* pCamera);

	void DrawGUI();

private:
	//Device* m_Device{ nullptr };

	std::unique_ptr<VertexBuffer<Vertex>> m_VertexBuffer{ nullptr };
	std::unique_ptr<IndexBuffer> m_IndexBuffer{ nullptr };

	//std::unique_ptr<ConstantBuffer<cbPerObject>> m_ConstantBuffer{ nullptr };
	cbPerObject m_cbData{};
	std::vector<ConstantBuffer<cbPerObject>> m_ConstantBuffers;
	std::vector<cbPerObject> m_cbDatas{};
protected:
	// Transforms
	void UpdateWorld();
	XMMATRIX m_WorldMatrix	{ XMMatrixIdentity() };
	XMVECTOR m_Translation	{ XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
	XMVECTOR m_Rotation		{ XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f) };
	XMVECTOR m_Scale		{ XMVectorSet(1.0f, 1.0f, 1.0f, 0.0) };
	// For GUI usage
	std::array<float, 3> m_Translations	{ 0.0f, 0.0f, 0.0f };
	std::array<float, 3> m_Rotations	{ 0.0f, 0.0f, 0.0f };
	std::array<float, 3> m_Scales		{ 1.0f, 1.0f, 1.0f };

	bool bInitialized{ false };
};

