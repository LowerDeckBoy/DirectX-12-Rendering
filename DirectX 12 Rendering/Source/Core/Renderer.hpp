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

	inline Window* GetWindow() const { return m_Window.get(); }
private:
	std::unique_ptr<Window> m_Window;

private:
	// Triangle resources
	ComPtr<ID3D12PipelineState> m_PipelineState;
	void InitTriangle();
	ComPtr<ID3D12Resource> m_VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_VertexView;
	//struct TriangleVertex
	//{
	//	XMFLOAT3 Position;
	//	XMFLOAT4 Color;
	//};
	struct VertexUV
	{
		XMFLOAT3 Position;
		XMFLOAT2 TexCoord;
	};

	//https://alextardif.com/D3D11To12P3.html
	void LoadAssets(const std::string& TexturePath);
	ComPtr<ID3D12Resource> m_TriangleTexture;
};

