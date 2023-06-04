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

#include "../Rendering/Model/Model.hpp"

#include "ComputePipelineState.hpp"

#include "../Graphics/Skybox.hpp"

class Camera;

using namespace DirectX;

//: public Device
class Renderer 
{
public:
	//explicit Renderer(HINSTANCE hInstance);
	~Renderer();

	void Initialize(Camera& refCamera);
	void LoadAssets();

	void PreRender();
	void Update(XMMATRIX ViewProj);
	void Draw(Camera* pCamera);

	void RecordCommandLists(uint32_t CurrentFrame, Camera* pCamera);

	void OnResize();
	void ResizeBackbuffers();

	void OnDestroy();

	Device* GetDeviceContext() { return m_Device.get(); }

protected:
	void SetRenderTarget();
	void ClearRenderTarget(CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle);

	void TransitToRender();
	void TransitToPresent();

	ID3D12PipelineState* SetPSO(int32_t Selected = 0);

private:
	std::unique_ptr<Device> m_Device;
	std::unique_ptr<GUI> m_GUI;

private:
	std::array<const float, 4> m_ClearColor{ 0.5f, 0.5f, 1.0f, 1.0f };

	// DepthStencil
	inline ID3D12DescriptorHeap* GetDepthHeap() const { return m_DepthHeap.Get(); };
	ComPtr<ID3D12Resource> m_DepthStencil;
	ComPtr<ID3D12DescriptorHeap> m_DepthHeap;
	void CreateDepthStencil();

	ComPtr<ID3D12RootSignature> m_ModelRootSignature;
	// PSO
	ComPtr<ID3D12PipelineState> m_ModelPipelineState;
	ComPtr<ID3D12PipelineState> m_PBRPipelineState;
	ComPtr<ID3D12PipelineState> m_PBRPipelineState2;
	ComPtr<ID3D12PipelineState> m_PBRPipelineState3;
	static inline int m_SelectedPSO = 0;
	
	void SwitchPSO();
	void InitModelPipeline();

	std::unique_ptr<Model> m_Model;

	// Skybox
	ComPtr<ID3D12RootSignature> m_SkyboxRootSignature;
	ComPtr<ID3D12PipelineState> m_SkyboxPipelineState;
	Skybox* m_Skybox;

};
