#include "../Graphics/Buffer.hpp"
#include "ScreenQuad.hpp"
#include <array>


void ScreenQuad::Create(DeviceContext* pDevice)
{
	std::array<ScreenQuadVertex, 4> Vertices =
	{
		ScreenQuadVertex{ { -1.0f, +1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
		ScreenQuadVertex{ { +1.0f, +1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
		ScreenQuadVertex{ { +1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
		ScreenQuadVertex{ { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } }
	};

	std::array<uint32_t, 6> Indices =
	{
		0, 1, 2, 2, 3, 0
	};

	const auto heapType{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
	m_VertexBuffer.Create(pDevice, BufferData(Vertices.data(), Vertices.size(), Vertices.size() * sizeof(ScreenQuadVertex), sizeof(Vertices.at(0))), BufferDesc(heapType, D3D12_RESOURCE_STATE_GENERIC_READ));

	m_IndexBuffer.Create(pDevice, BufferData(Indices.data(), Indices.size(), Indices.size() * sizeof(uint32_t), sizeof(uint32_t)), BufferDesc(heapType, D3D12_RESOURCE_STATE_GENERIC_READ));

}

void ScreenQuad::Draw(ID3D12GraphicsCommandList* pCommandList)
{
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetVertexBuffers(0, 1, &m_VertexBuffer.View);
	pCommandList->IASetIndexBuffer(&m_IndexBuffer.View);
	pCommandList->DrawIndexedInstanced(m_IndexBuffer.Count, 1, 0, 0, 0);
}