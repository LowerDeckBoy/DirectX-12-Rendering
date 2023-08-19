#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "../Graphics/ConstantBuffer.hpp"
#include "ScreenQuad.hpp"

class DeviceContext;
class Camera;
struct Descriptor;
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

	// GUI
	void DrawDeferredTargets();

public:
	void CreateRenderTargets();
	void CreatePipelines(ShaderManager* pShaderManager, ID3D12RootSignature* pModelRootSignature);

	// For Deferred Render Targets only
	ComPtr<ID3D12DescriptorHeap> m_DeferredHeap;

	std::array<ComPtr<ID3D12Resource>, RenderTargetsCount> m_RenderTargets;
	std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, RenderTargetsCount> m_RenderTargetDescriptors;
	std::array<Descriptor, RenderTargetsCount> m_ShaderDescriptors;

private:
	void BeginPass();
	void EndPass();

	void Release();

private:
	DeviceContext* m_DeviceCtx{ nullptr };

	std::array<float, 4> m_ClearColor{ 0.5f, 0.5f, 1.0f, 1.0f };
	DXGI_FORMAT m_DepthFormat{ DXGI_FORMAT_D32_FLOAT };

	ComPtr<ID3D12RootSignature> m_RootSignature;
	ComPtr<ID3D12PipelineState> m_PipelineState;
	ComPtr<ID3D12PipelineState> m_LightPipelineState;

	const std::array<DXGI_FORMAT, RenderTargetsCount> m_RenderTargetFormats{ 
		DXGI_FORMAT_R8G8B8A8_UNORM,			// Base Color
		DXGI_FORMAT_R16G16B16A16_FLOAT,		// Normal
		DXGI_FORMAT_R8G8B8A8_UNORM,			// Metal-Roughness
		DXGI_FORMAT_R16G16B16A16_FLOAT,		// Emissive
		DXGI_FORMAT_R32G32B32A32_FLOAT,		// World Position
		DXGI_FORMAT_R8G8B8A8_UNORM			// Depth
	};
};
