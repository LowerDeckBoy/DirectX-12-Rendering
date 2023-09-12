#pragma once
#include <d3d12.h>
#include "../Graphics/Buffer/ConstantBuffer.hpp"
#include "ShadowMap.hpp"
#include "ScreenQuad.hpp"

class DeviceContext;
class Camera;
class Descriptor;
class ShaderManager;
class Model;
class ImageBasedLighting;
class PointLights;

class DeferredContext : public ScreenQuad
{
public:
	DeferredContext() {}
	DeferredContext(DeviceContext* pDeviceContext, ShaderManager* pShaderManager, ID3D12RootSignature* pModelRootSignature);
	~DeferredContext();

	static const uint32_t RenderTargetsCount{ 6 };

	void Create(DeviceContext* pDeviceContext, ShaderManager* pShaderManager, ID3D12RootSignature* pModelRootSignature);

	void OnResize();

	void PassGBuffer(Camera* pCamera, ConstantBuffer<SceneConstData>* CameraCB, std::vector<std::unique_ptr<Model>>& Models);
	void PassLight(Camera* pCamera, ConstantBuffer<SceneConstData>* CameraCB, ImageBasedLighting* pIBL, PointLights* pPointLights);
	void PassShadows(Camera* pCamera, ConstantBuffer<SceneConstData>* CameraCB, PointLights* pPointLights, ImageBasedLighting* pIBL, std::vector<std::unique_ptr<Model>>& Models);

	// GUI
	void DrawDeferredTargets();

public:
	void CreateRenderTargets();
	void CreatePipelines(ShaderManager* pShaderManager, ID3D12RootSignature* pModelRootSignature);
	void DrawToShadowMap(Camera* pCamera, ConstantBuffer<SceneConstData>* CameraCB, PointLights* pPointLights, std::vector<std::unique_ptr<Model>>& Models, ID3D12RootSignature* pModelRootSignature);

	// For Deferred Render Targets only
	ComPtr<ID3D12DescriptorHeap> m_DeferredHeap;
	ComPtr<ID3D12DescriptorHeap> m_DeferredDepthHeap;

	std::array<ComPtr<ID3D12Resource>, RenderTargetsCount> m_RenderTargets;
	std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, RenderTargetsCount> m_RenderTargetDescriptors;
	// Shader Resource Views
	std::array<Descriptor, RenderTargetsCount> m_ShaderDescriptors;

	inline const ShadowMap& GetShadowMap() const { return *m_ShadowMap.get(); }

private:
	void Release();

	DeviceContext* m_DeviceCtx{ nullptr };

	std::array<float, 4> m_ClearColor{ 0.5f, 0.5f, 1.0f, 1.0f };
	DXGI_FORMAT m_DepthFormat{ DXGI_FORMAT_D32_FLOAT };

	ComPtr<ID3D12RootSignature> m_RootSignature;
	ComPtr<ID3D12RootSignature> m_ShadowRootSignature;

	// G-Buffer PSO
	ComPtr<ID3D12PipelineState> m_PipelineState;
	// Post G-Buffer Lighting
	ComPtr<ID3D12PipelineState> m_LightPipelineState;
	//
	ComPtr<ID3D12PipelineState> m_ShadowPipelineState;
	// Writing to Depth Map and clipping alpha
	ComPtr<ID3D12PipelineState> m_PreShadowPipelineState;


	const std::array<DXGI_FORMAT, RenderTargetsCount> m_RenderTargetFormats{ 
		DXGI_FORMAT_R8G8B8A8_UNORM,			// Base Color
		DXGI_FORMAT_R16G16B16A16_FLOAT,		// Normal
		DXGI_FORMAT_R8G8B8A8_UNORM,			// Metal-Roughness
		DXGI_FORMAT_R16G16B16A16_FLOAT,		// Emissive
		DXGI_FORMAT_R32G32B32A32_FLOAT,		// World Position
		DXGI_FORMAT_R8G8B8A8_UNORM			// Depth
	};

	std::unique_ptr<ShadowMap> m_ShadowMap;
	
};
