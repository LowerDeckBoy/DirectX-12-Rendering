#pragma once
#include "DeviceContext.hpp"
#include <memory>
#include "../Graphics/ShaderManager.hpp"
#include "../Graphics/Buffer/ConstantBuffer.hpp"
#include "../Editor/Editor.hpp"
#include "../Rendering/Model/Model.hpp"
#include "../Rendering/DeferredContext.hpp"
#include "../Raytracing/RaytracingContext.hpp"
#include "../Graphics/ImageBasedLighting.hpp"
#include "../Rendering/Light/PointLights.hpp"

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
	void Render();
	void DrawSkybox();
	void Forward();
	void Deferred();

	void RecordCommandLists(uint32_t CurrentFrame);

	void OnResize();

	void Release();

	inline DeviceContext* GetDeviceContext() noexcept { return m_DeviceCtx.get(); }

protected:
	void BeginFrame();
	void EndFrame();

	// Wrappers
	void SetHeap(ID3D12DescriptorHeap** ppHeap);
	void SetRootSignature(ID3D12RootSignature* pRootSignature);
	void SetPipelineState(ID3D12PipelineState* pPipelineState);

	void SetViewport();

	void SetRenderTarget();
	void ClearRenderTarget();
	void ClearDepthStencil();

	void TransitToRender();
	void TransitToPresent(D3D12_RESOURCE_STATES StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET);

	ID3D12PipelineState* SetPSO(int32_t Selected = 0) noexcept;

private:
	std::unique_ptr<DeviceContext> m_DeviceCtx;

	Camera* m_Camera{ nullptr };

	std::unique_ptr<Editor> m_GUI;

private:
	std::array<const float, 4> m_ClearColor{ 0.5f, 0.5f, 1.0f, 1.0f };

	ComPtr<ID3D12RootSignature> m_ModelRootSignature;
	
	// PSO
	ComPtr<ID3D12PipelineState> m_PBRPipelineState;
	ComPtr<ID3D12PipelineState> m_IBLPipelineState;

	static inline int m_SelectedPSO{ 0 };
	
	void SwitchPSO();

	void InitPipelines();

	std::vector<std::unique_ptr<Model>> m_Models;

	// Skybox
	ComPtr<ID3D12RootSignature> m_SkyboxRootSignature;
	ComPtr<ID3D12PipelineState> m_SkyboxPipelineState;
	//std::unique_ptr<Skybox> m_Skybox;
	// IBL uses the same Pipeline State and Root Signature as Skybox
	std::unique_ptr<ImageBasedLighting> m_IBL;

	std::shared_ptr<ShaderManager> m_ShaderManager;

	// Deferred Context
	std::unique_ptr<DeferredContext> m_DeferredCtx;

	// Deferred CBVs
	// Camera Scene data
	std::unique_ptr<ConstantBuffer<SceneConstData>> m_cbCamera;
	SceneConstData m_cbSceneData{};
	
	void DrawGUI();

	std::unique_ptr<PointLights> m_PointLights;

	// TODO: RT
	std::unique_ptr<RaytracingContext> m_RaytracingContext;

public:
	static bool bVsync;
	static bool bDrawSky;
	static bool bDeferred;
	static bool bRaytrace;

};
