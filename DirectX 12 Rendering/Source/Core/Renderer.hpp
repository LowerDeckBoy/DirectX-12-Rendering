#pragma once
#include "Device.hpp"
#include "Window.hpp"
#include <memory>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "../Graphics/Shader.hpp"
#include "../Graphics/Buffer.hpp"


using namespace DirectX;
//: public Device
class Renderer 
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
	void WaitforGPU();

	void OnDestroy();

	inline Window* GetWindow() const { return m_Window.get(); }
private:
	std::unique_ptr<Window> m_Window;
	std::unique_ptr<Device> m_Device;

private:
	// Triangle resources
	ComPtr<ID3D12PipelineState> m_PipelineState;
	void InitTriangle();

	struct VertexUV
	{
		XMFLOAT3 Position;
		XMFLOAT2 TexCoord;
	};

	//test
	VertexBuffer<VertexUV> m_VertexBuffer;
	IndexBuffer m_IndexBuffer;
	

	//struct TriangleVertex
	//{
	//	XMFLOAT3 Position;
	//	XMFLOAT4 Color;
	//};

	void LoadAssets(const std::string& TexturePath);
	ComPtr<ID3D12Resource> m_TriangleTexture;

	// Shaders
	std::unique_ptr<VertexShader> m_VertexShader{ std::make_unique<VertexShader>() };
	std::unique_ptr<PixelShader> m_PixelShader{ std::make_unique<PixelShader>() };

	//


};

