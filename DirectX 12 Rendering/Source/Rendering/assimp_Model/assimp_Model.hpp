#pragma once
#include "asssimp_Importer.hpp"
#include "../../Graphics/Buffer.hpp"
#include "../../Graphics/ConstantBuffer.hpp"
#include <memory>
#include <array>

class Camera;
class Device;

class assimp_Model : public asssimp_Importer
{
public:

	void Create(Device* pDevice, std::string_view Filepath);
	void Draw(Camera* pCamera);

	void DrawGUI();

private:

	std::unique_ptr<VertexBuffer<Vertex>> m_VertexBuffer;
	std::unique_ptr<IndexBuffer> m_IndexBuffer;
	std::unique_ptr<ConstantBuffer<cbPerObject>> m_ConstBuffer;
	cbPerObject m_cbData;
		
protected:
	void UpdateWorld();
	XMMATRIX m_WorldMatrix	{ XMMatrixIdentity() };
	XMVECTOR m_Translation	{ XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
	XMVECTOR m_Rotation		{ XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f) };
	XMVECTOR m_Scale		{ XMVectorSet(1.0f, 1.0f, 1.0f, 0.0) };
	// For GUI usage
	std::array<float, 3> m_Translations = { 0.0f, 0.0f, 0.0f };
	std::array<float, 3> m_Rotations	= { 0.0f, 0.0f, 0.0f };
	std::array<float, 3> m_Scales		= { 1.0f, 1.0f, 1.0f };
};

