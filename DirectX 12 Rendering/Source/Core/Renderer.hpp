#pragma once
#include "Device.hpp"
#include <memory>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "../Graphics/Descriptors.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Buffer.hpp"

#include "../Editor/GUI.hpp"

using namespace DirectX;
//: public Device
class Renderer 
{
public:
	//explicit Renderer(HINSTANCE hInstance);
	~Renderer();

	void Initialize();
	
	void InitPipelineState();

	void Update();
	void Draw();

	void RecordCommandLists();
	//void WaitForPreviousFrame();

	void OnResize();
	void ResizeBackbuffers();
	// Temporal
	void FlushGPU();

	void MoveToNextFrame();
	void WaitForGPU();

	void OnDestroy();

private:
	std::unique_ptr<Device> m_Device;

	//test
	std::unique_ptr<GUI> m_GUI;

private:
	std::array<const float, 4> m_ClearColor{ 0.5f, 0.5f, 1.0f, 1.0f };

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

	// DepthStencil
	inline ID3D12DescriptorHeap* GetDepthHeap() const { return m_DepthHeap.Get(); };
	ComPtr<ID3D12Resource> m_DepthStencil;
	ComPtr<ID3D12DescriptorHeap> m_DepthHeap;
	void CreateDepthStencil();

};

