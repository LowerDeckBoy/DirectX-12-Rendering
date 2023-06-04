#include "../Core/Device.hpp"
#include "../Graphics/Buffer.hpp"
#include "../Graphics/ConstantBuffer.hpp"
#include "Cube.hpp"

Cube::~Cube()
{
	Release();
}

void Cube::Initialize(Device* pDevice)
{
	assert(m_Device = pDevice);

	m_VertexBuffer.Create(pDevice, *m_Vertices);
	m_IndexBuffer.Create(pDevice, *m_Indices);
	m_ConstBuffer.Create(pDevice, &m_cbData);
}

void Cube::Draw(DirectX::XMMATRIX ViewProjection)
{
	//m_Device->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//m_Device->GetCommandList()->IASetVertexBuffers(0, 1, &m_VertexBuffer.GetBufferView());
	//m_Device->GetCommandList()->IASetIndexBuffer(&m_IndexBuffer.GetBufferView());
	//m_Device->GetCommandList()->DrawIndexedInstanced(m_IndexBuffer.GetSize(), 1, 0, 0, 0);

	m_ConstBuffer.Update({ DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity * ViewProjection) }, m_Device->m_FrameIndex);
	m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(0, m_ConstBuffer.GetBuffer(m_Device->m_FrameIndex)->GetGPUVirtualAddress());

	m_Device->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Device->GetCommandList()->IASetVertexBuffers(0, 1, &m_VertexBuffer.View.BufferLocation);
	m_Device->GetCommandList()->IASetIndexBuffer(&m_IndexBuffer.View.BufferLocation);
	m_Device->GetCommandList()->DrawIndexedInstanced(m_IndexBuffer.Count(), 1, 0, 0, 0);
}

void Cube::Release()
{
	delete m_Vertices;
	delete m_Indices;
}
