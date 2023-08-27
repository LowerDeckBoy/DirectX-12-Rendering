#pragma once
#include "../Core/DeviceContext.hpp"
#include "Buffer/Buffer.hpp"
#include "Buffer/ConstantBuffer.hpp"
#include "Buffer/Vertex.hpp"

class Camera;
class Texture;

class Skybox
{
public:
	Skybox() { }
	Skybox(DeviceContext* pDevice);

	void Create(DeviceContext* pDevice);
	void Draw(Camera* pCamera);
	void UpdateWorld(Camera* pCamera);
	void Release();

	inline ID3D12Resource* GetTexture() { return m_Texture->GetTexture(); }
	Texture GetTex();

private:
	DeviceContext* m_Device{ nullptr };
	std::unique_ptr<VertexBuffer> m_VertexBuffer{ nullptr };
	std::unique_ptr<IndexBuffer> m_IndexBuffer{ nullptr };
	std::unique_ptr<ConstantBuffer<cbPerObject>> m_ConstBuffer{ nullptr };
	cbPerObject m_cbData{};

	std::unique_ptr<Texture> m_Texture{ nullptr };

	XMMATRIX m_WorldMatrix	{ XMMatrixIdentity() };
	XMVECTOR m_Translation	{ XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
	XMVECTOR m_Scale		{ XMVectorSet(5000.0f, 5000.0f, 5000.0f, 0.0f) };
};