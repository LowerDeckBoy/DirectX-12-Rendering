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

class DeferredContext
{
public:
	DeferredContext();
	DeferredContext(DeviceContext* pDeviceContext, ShaderManager* pShaderManager, ID3D12RootSignature* pModelRootSignature);
	~DeferredContext();

	static const uint32_t DeferredRTVCount{ 6 };

	void Create(DeviceContext* pDeviceContext, ShaderManager* pShaderManager, ID3D12RootSignature* pModelRootSignature);

	void OnResize();

	void PassGBuffer(Camera* pCamera, ConstantBuffer<SceneConstData>* CameraCB, std::vector<Model*>& Models);
	void PassLight(Camera* pCamera);

	// GUI
	void DrawDeferredTargets();

public:
	void CreateRenderTargets();
	void CreatePipelines(ShaderManager* pShaderManager, ID3D12RootSignature* pModelRootSignature);

	// For Deferred Render Targets
	ComPtr<ID3D12DescriptorHeap> m_DeferredHeap;

	std::array<ComPtr<ID3D12Resource>, DeferredRTVCount> m_RenderTargets;
	std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, DeferredRTVCount> m_RenderTargetDescriptors;
	std::array<Descriptor, DeferredRTVCount> m_ShaderDescriptors;

private:
	DeviceContext* m_DeviceCtx{ nullptr };
	std::unique_ptr<ScreenQuad> m_ScreenQuad;
	std::array<float, 4> m_ClearColor{ 0.5f, 0.5f, 1.0f, 1.0f };

	ComPtr<ID3D12RootSignature> m_RootSignature;
	ComPtr<ID3D12PipelineState> m_PipelineState;
	ComPtr<ID3D12PipelineState> m_LightPipelineState;

	std::array<DXGI_FORMAT, DeferredRTVCount> m_RTVFormats{ DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8B8A8_UNORM };


};

