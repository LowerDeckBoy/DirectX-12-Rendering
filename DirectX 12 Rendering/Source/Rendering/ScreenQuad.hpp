#pragma once
//#include <d3d12.h>

struct ID3D12GraphicsCommandList;
class DeviceContext;
class VertexBuffer;
class IndexBuffer;

// Quad for displaying Deferred output
class ScreenQuad
{
public:
	ScreenQuad(DeviceContext* pDeviceCtx);
	~ScreenQuad();

	void Create(DeviceContext* pDeviceCtx);
	void Draw(ID3D12GraphicsCommandList* pCommandList);

	void Release();

private:
	VertexBuffer* m_VertexBuffer{ nullptr };
	IndexBuffer* m_IndexBuffer{ nullptr };

};
