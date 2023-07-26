#pragma once
#include "../Core/DeviceContext.hpp"
#include "../Graphics/Buffer.hpp"
#include "../Graphics/ConstantBuffer.hpp"
#include "Vertex.hpp"

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

	ID3D12Resource* GetTexture() { return m_Texture->GetTexture(); }
	Texture GetTex();

private:
	DeviceContext* m_Device{ nullptr };
	std::unique_ptr<VertexBuffer> m_VertexBuffer{ nullptr };
	std::unique_ptr<IndexBuffer> m_IndexBuffer{ nullptr };
	std::unique_ptr<ConstantBuffer<cbPerObject>> m_ConstBuffer{ nullptr };
	cbPerObject m_cbData{};

	std::unique_ptr<Texture> m_Texture{ nullptr };

	std::vector<SkyboxVertex>* m_Vertices = new std::vector<SkyboxVertex>{
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) }
	};

	std::vector<uint32_t>* m_Indices = new std::vector<uint32_t>{
		0, 1, 2, 0, 2, 3,
		4, 6, 5, 4, 7, 6,
		4, 5, 1, 4, 1, 0,
		3, 2, 6, 3, 6, 7,
		1, 5, 6, 1, 6, 2,
		4, 0, 3, 4, 3, 7
	};

	XMMATRIX m_WorldMatrix	{ XMMatrixIdentity() };
	XMVECTOR m_Translation	{ XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
	XMVECTOR m_Scale		{ XMVectorSet(5000.0f, 5000.0f, 5000.0f, 0.0f) };
};