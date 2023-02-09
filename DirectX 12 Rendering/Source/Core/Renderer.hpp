#pragma once
#include "Device.hpp"
#include "Window.hpp"
#include <memory>
#include <DirectXMath.h>
#include <d3dcompiler.h>

using namespace DirectX;

class Renderer : public Device
{
public:
	explicit Renderer(HINSTANCE hInstance);
	~Renderer();

	void Initialize();
	
	void InitPipelineState();

	void Update();
	void Draw();

	void RecordCommandLists();
	void WaitForPreviousFrame();

	void OnDestroy();

private:
	std::unique_ptr<Window> m_Window;

private:
	//https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloTriangle/D3D12HelloTriangle.cpp
	// Triangle resources
	ComPtr<ID3D12PipelineState> m_PipelineState;
	void InitTriangle();
	ComPtr<ID3D12Resource> m_VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_VertexView;
	struct TriangleVertex
	{
		XMFLOAT3 Position;
		XMFLOAT4 Color;
	} m_TriangleVertex;

};

