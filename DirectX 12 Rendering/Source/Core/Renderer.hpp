#pragma once
#include "DeviceContext.hpp"
#include <memory>
#include "../Graphics/ShaderManager.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/ConstantBuffer.hpp"
#include "../Editor/Editor.hpp"
#include "ComputePipelineState.hpp"
#include "../Rendering/Model/Model.hpp"
#include "../Graphics/Skybox.hpp"
#include "../Rendering/ScreenQuad.hpp"
#include "../Utilities/Logger.hpp"
#include "../Graphics/ImageBasedLighting.hpp"

class Camera;

using namespace DirectX;

class Renderer 
{
public:
	~Renderer();

	void Initialize(Camera* pCamera);
	void LoadAssets();

	void PreRender();
	void Update();
	void Draw();
	void DrawSkybox();
	void Forward();
	void Deferred();

	void RecordCommandLists(uint32_t CurrentFrame);

	void OnResize();

	void Release();

	inline DeviceContext* GetDeviceContext() noexcept { return m_Device.get(); }

protected:
	// Wrappers
	void SetHeap(ID3D12DescriptorHeap** ppHeap);
	void SetRootSignature(ID3D12RootSignature* pRootSignature);
	void SetPipelineState(ID3D12PipelineState* pPipelineState);

	void SetRenderTarget();
	void ClearRenderTarget();
	void ClearDepthStencil();

	void TransitToRender();
	void TransitToPresent(D3D12_RESOURCE_STATES StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET);

	ID3D12PipelineState* SetPSO(int32_t Selected = 0) noexcept;

	void BeginFrame();
	void EndFrame();

private:
	std::unique_ptr<DeviceContext> m_Device;

	Camera* m_Camera{ nullptr };

	std::unique_ptr<Editor> m_GUI;

private:
	std::array<const float, 4> m_ClearColor{ 0.5f, 0.5f, 1.0f, 1.0f };

	ComPtr<ID3D12RootSignature> m_ModelRootSignature;
	ComPtr<ID3D12RootSignature> m_DeferredRootSignature;
	// PSO
	ComPtr<ID3D12PipelineState> m_ModelPipelineState;
	ComPtr<ID3D12PipelineState> m_PBRPipelineState;
	ComPtr<ID3D12PipelineState> m_PBRPipelineState2;
	ComPtr<ID3D12PipelineState> m_PBRPipelineState3;
	static inline int m_SelectedPSO = 0;
	
	void SwitchPSO();
	// TODO: Requires some cleanup
	void InitPipelines();

	std::unique_ptr<Model> m_Model;
	std::unique_ptr<Model> m_Model2;

	// Skybox
	ComPtr<ID3D12RootSignature> m_SkyboxRootSignature;
	ComPtr<ID3D12PipelineState> m_SkyboxPipelineState;
	std::unique_ptr<Skybox> m_Skybox;
	std::unique_ptr<ImageBasedLighting> m_IBL;

	std::unique_ptr<ShaderManager> m_ShaderManager;

	// Deferred Context
	ScreenQuad m_ScreenQuad;

	static const int32_t m_DeferredRTVCount{ 6 };
	void CreateDeferredRTVs();
	ComPtr<ID3D12DescriptorHeap> m_DefRTVHeap;
	ComPtr<ID3D12Resource> m_RTVTextures[m_DeferredRTVCount];
public:
	std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, m_DeferredRTVCount> m_RTVDescriptors;
	std::array<Descriptor, m_DeferredRTVCount> m_RTSRVDescs;
private:
	ComPtr<ID3D12PipelineState> m_DeferredPSO;
	ComPtr<ID3D12PipelineState> m_DeferredLightPSO;

	std::array<DXGI_FORMAT, m_DeferredRTVCount> m_RTVFormats{ DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM };

	// GBuffer
	void PassGBuffer(Camera* pCamera);
	void PassLight(Camera* pCamera);

	// Deferred CBVs
	// Camera Scene data
	std::unique_ptr<ConstantBuffer<SceneConstData>> m_cbCamera;
	SceneConstData m_cbSceneData{};
	// Data for light shading
	std::unique_ptr<ConstantBuffer<cbMaterial>> m_cbMaterial;
	cbMaterial m_cbMaterialData{};
	
	void DrawGUI();

	// Lighting
	// Temporal
	void SetupLights();
	void UpdateLights();
	void ResetLights();

	// Const buffer for light positions and colors -> PBR
	std::unique_ptr<ConstantBuffer<cbLights>> m_cbPointLights;
	cbLights m_cbPointLightsData{};

	//Light positions
	std::array<XMFLOAT4, 4>				m_LightPositions;
	std::array<std::array<float, 4>, 4> m_LightPositionsFloat;
	std::array<XMFLOAT4, 4>				m_LightColors;
	std::array<std::array<float, 4>, 4> m_LightColorsFloat;

public:
	static bool bDrawSky;
	static bool bDeferred;

	Logger m_Logger;

};
