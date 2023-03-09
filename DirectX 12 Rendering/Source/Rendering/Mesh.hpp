#pragma once

#include "Primitive.hpp"

class Device;

struct Mesh
{
public:
	Mesh() { }
	explicit Mesh(Device* pDevice) : m_Device(pDevice) { }

	Mesh(Device* pDevice, std::vector<Vertex>& Vertices, std::vector<uint32_t>& Indices) 
		: m_Vertices(Vertices), m_Indices(Indices)
	{
		assert(m_Device = pDevice);

		m_VertexBuffer = std::make_unique<VertexBuffer<Vertex>>();
		m_IndexBuffer = std::make_unique<IndexBuffer>();

		m_VertexBuffer->Create(pDevice, m_Vertices);
		m_IndexBuffer->Create(pDevice, m_Indices);
	}
	//Mesh(Device* pDevice);
	Mesh(const Mesh& rhs)
	{
		Primitives = rhs.Primitives;
		m_Matrix = rhs.m_Matrix;
	}
	Mesh& operator=(const Mesh& rhs)
	{
		//Mesh* newMesh{ new Mesh() };
		//newMesh->Primitives = rhs.Primitives;
		//newMesh->m_Matrix = rhs.m_Matrix;
		//return *newMesh;
		return *this;
	}

	~Mesh()
	{
		//Release();
	}

	void SetVertices(Device* pDevice, std::vector<Vertex>& Vertices)
	{
		m_Vertices = Vertices;
		m_VertexBuffer = std::make_unique<VertexBuffer<Vertex>>();
		m_VertexBuffer->Create(pDevice, m_Vertices);
	}


	void SetIndices(Device* pDevice, std::vector<uint32_t>& Indices)
	{
		m_Indices = Indices;
		m_IndexBuffer = std::make_unique<IndexBuffer>();
		m_IndexBuffer->Create(pDevice, m_Indices);
	}

	// TEST
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	std::unique_ptr<VertexBuffer<Vertex>> m_VertexBuffer;
	std::unique_ptr<IndexBuffer> m_IndexBuffer;

	Device* m_Device;

	void Draw()
	{
		m_Device->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_Device->GetCommandList()->IASetVertexBuffers(0, 1, &m_VertexBuffer->GetBufferView());
		m_Device->GetCommandList()->IASetIndexBuffer(&m_IndexBuffer->GetBufferView());
		
		m_Device->GetCommandList()->DrawIndexedInstanced(m_IndexBuffer->GetSize(),
														 1, 0, 0, 0);
	}


	std::vector<Primitive*> Primitives;

	DirectX::XMMATRIX m_Matrix{ DirectX::XMMatrixIdentity() };

	void Release() 
	{
		for (auto& primitive : Primitives)
			delete primitive;
	}
};

