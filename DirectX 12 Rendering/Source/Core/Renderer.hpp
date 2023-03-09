#pragma once
#include "Device.hpp"
#include <memory>
#include <DirectXMath.h>
#include <d3dcompiler.h>
//#include "../Graphics/Descriptors.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Buffer.hpp"
#include "../Graphics/ConstantBuffer.hpp"
#include "../Graphics/Cube.hpp"
#include "../Graphics/Texture.hpp"

#include "../Editor/GUI.hpp"

#include "../Rendering/Model.hpp"


class Camera;

using namespace DirectX;

struct cBuffer
{
	DirectX::XMFLOAT4 Offset;
	float padding[60];
};


//: public Device
class Renderer 
{
public:
	//explicit Renderer(HINSTANCE hInstance);
	~Renderer();

	void Initialize(Camera& refCamera);
	
	void InitPipelineState();

	//void Update(XMMATRIX ViewProj);
	//void Draw(XMMATRIX ViewProjection);
	void Update(XMMATRIX ViewProj);
	void Draw(Camera* pCamera);

	//void RecordCommandLists(XMMATRIX ViewProjection);
	void RecordCommandLists(Camera* pCamera);

	//void WaitForPreviousFrame();

	void OnResize();
	void ResizeBackbuffers();

	// Temporal
	void FlushGPU();

	void MoveToNextFrame();
	void WaitForGPU();

	void OnDestroy();

protected:
	void SetRenderTarget();
	void ClearRenderTarget(CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle);

	void TransitToRender();
	void TransitToPresent();

private:
	std::unique_ptr<Device> m_Device;
	//test
	std::unique_ptr<GUI> m_GUI;

private:
	std::array<const float, 4> m_ClearColor{ 0.5f, 0.5f, 1.0f, 1.0f };

	struct VertexUV
	{
		XMFLOAT3 Position;
		XMFLOAT2 TexCoord;
	};
	struct CubeVertex
	{
		XMFLOAT3 Position;
		XMFLOAT4 Color;
	};


	VertexBuffer<CubeVertex> m_VertexBuffer;
	IndexBuffer m_IndexBuffer;
	
	//void LoadAssets(const std::string& TexturePath);
	//ComPtr<ID3D12Resource> m_TriangleTexture;

	// Shaders
	std::unique_ptr<VertexShader> m_VertexShader{ std::make_unique<VertexShader>() };
	std::unique_ptr<PixelShader> m_PixelShader{ std::make_unique<PixelShader>() };

	// DepthStencil
	inline ID3D12DescriptorHeap* GetDepthHeap() const { return m_DepthHeap.Get(); };
	ComPtr<ID3D12Resource> m_DepthStencil;
	ComPtr<ID3D12DescriptorHeap> m_DepthHeap;
	void CreateDepthStencil();

	// HEAP
	// Main Descriptor Heap
	ComPtr<ID3D12DescriptorHeap> m_MainHeap;
	

	// TEST
	//Cube m_Cube;
	//Texture m_Texture;
	ComPtr<ID3D12RootSignature> m_ModelRootSignature;
	ComPtr<ID3D12PipelineState> m_ModelPipelineState;
	void InitModelPipeline();
	Model m_Model;
};
