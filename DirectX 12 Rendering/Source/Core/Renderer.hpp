#pragma once
#include "Device.hpp"
#include <memory>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "../Graphics/Shader.hpp"
#include "../Graphics/Buffer.hpp"
#include "../Graphics/ConstantBuffer.hpp"
#include "../Graphics/Cube.hpp"
#include "../Graphics/Texture.hpp"

#include "../Editor/GUI.hpp"

#include "../Rendering/cglTF_Model/cglTF_Model.hpp"
#include "../Rendering/assimp_Model/assimp_Model.hpp"

#include "../Graphics/Skybox.hpp"

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
	void LoadAssets();

	void Update(XMMATRIX ViewProj);
	void Draw(Camera* pCamera);

	void RecordCommandLists(uint32_t CurrentFrame, Camera* pCamera);

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
	std::unique_ptr<GUI> m_GUI;

private:
	std::array<const float, 4> m_ClearColor{ 0.5f, 0.5f, 1.0f, 1.0f };

	// Shaders
	std::unique_ptr<Shader> m_VertexShader{ std::make_unique<Shader>() };
	std::unique_ptr<Shader> m_PixelShader{ std::make_unique<Shader>() };

	// DepthStencil
	inline ID3D12DescriptorHeap* GetDepthHeap() const { return m_DepthHeap.Get(); };
	ComPtr<ID3D12Resource> m_DepthStencil;
	ComPtr<ID3D12DescriptorHeap> m_DepthHeap;
	void CreateDepthStencil();

	// Main Descriptor Heap
	ComPtr<ID3D12DescriptorHeap> m_MainHeap;
	
	ComPtr<ID3D12RootSignature> m_ModelRootSignature;
	ComPtr<ID3D12PipelineState> m_ModelPipelineState;
	void InitModelPipeline();
	//

	//cglTF_Model m_TestLoader;
	assimp_Model m_Model;

	// Skybox
	ComPtr<ID3D12RootSignature> m_SkyboxRootSignature;
	ComPtr<ID3D12PipelineState> m_SkyboxPipelineState;
	Skybox m_Skybox;
	std::unique_ptr<Shader> m_SkyboxVS{ std::make_unique<Shader>() };
	std::unique_ptr<Shader> m_SkyboxPS{ std::make_unique<Shader>() };
};
