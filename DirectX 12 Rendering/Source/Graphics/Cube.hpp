#pragma once
//#include "../Core/Device.hpp"
//#include "../Graphics/Buffer.hpp"
//#include "../Graphics/ConstantBuffer.hpp"
#include "Vertex.hpp"
//#include <DirectXMath.h>

class Device;

class Cube
{
public:
	Cube() {}
	~Cube();

	void Initialize(Device* pDevice);
	void Draw(DirectX::XMMATRIX ViewProjection);
	void Release();

private:
	Device* m_Device;
	//VertexBuffer<CubeVertex> m_VertexBuffer;
	VertexBuffer m_VertexBuffer;
	IndexBuffer m_IndexBuffer;

	cbPerObject m_cbData{};
	ConstantBuffer<cbPerObject> m_ConstBuffer;

	std::vector<CubeVertex>* m_Vertices = new std::vector<CubeVertex>{
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ DirectX::XMFLOAT3( 1.0f,  1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ DirectX::XMFLOAT3( 1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ DirectX::XMFLOAT3( 1.0f,  1.0f,  1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ DirectX::XMFLOAT3( 1.0f, -1.0f,  1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) }
	};

	std::vector<uint32_t>* m_Indices = new std::vector<uint32_t>{
		0, 1, 2, 0, 2, 3,
		4, 6, 5, 4, 7, 6,
		4, 5, 1, 4, 1, 0,
		3, 2, 6, 3, 6, 7,
		1, 5, 6, 1, 6, 2,
		4, 0, 3, 4, 3, 7
	};
};
