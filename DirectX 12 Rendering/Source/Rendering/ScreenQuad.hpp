#pragma once
#include <d3d12.h>
#include <wrl.h>

class DeviceContext;
class VertexBuffer;
class IndexBuffer;

// Quad for displaying Deferred output
class ScreenQuad
{
public:
	void Create(DeviceContext* pDevice);
	void Draw(ID3D12GraphicsCommandList* pCommandList);

private:
	VertexBuffer m_VertexBuffer;
	IndexBuffer	 m_IndexBuffer;

};

