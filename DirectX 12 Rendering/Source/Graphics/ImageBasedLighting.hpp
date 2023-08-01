#pragma once
//#include "../Core/ComputePipelineState.hpp"
#include "Buffer.hpp"
#include "ConstantBuffer.hpp"

class DeviceContext;
class Camera;
struct Descriptor;

class ImageBasedLighting
{
public:
	ImageBasedLighting() {}
	~ImageBasedLighting();

	void Create(DeviceContext* pDevice, const std::string_view& Filepath);
	

	void Draw(Camera* pCamera, uint32_t FrameIndex);

	void Release();

	ComPtr<ID3D12Resource> m_EnvironmentTexture;
	Descriptor m_EnvDescriptor;
	//ComPtr<ID3D12Resource> m_IrradianceMap;
	//Descriptor m_IrradianceDescriptor;
	ComPtr<ID3D12Resource> m_PrefilteredMap;
	Descriptor m_PrefilteredDescriptor;

	Descriptor m_Descriptor;
	ComPtr<ID3D12Resource> m_OutputResource;
	Descriptor m_OutputDescriptor;

private:
	void InitializeTextures(DeviceContext* pDevice, const std::string_view& Filepath);
	void InitializeBuffers(DeviceContext* pDevice);

	void CreatePipeline();

	void CreateCubeTexture(DeviceContext* pDeviceCtx, const std::string_view& Filepath);
	void PrefilterSpecular(DeviceContext* pDeviceCtx, ID3D12RootSignature* pComputeRoot);

	ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	ComPtr<ID3D12PipelineState> m_Pipeline;

	VertexBuffer m_VertexBuffer;
	IndexBuffer m_IndexBuffer;
	ConstantBuffer<cbPerObject> m_ConstBuffer;
	cbPerObject m_cbData{};

	void UpdateWorld(Camera* pCamera);

	XMMATRIX m_WorldMatrix{ XMMatrixIdentity() };
	XMVECTOR m_Translation{ XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
	XMVECTOR m_Scale{ XMVectorSet(500.0f, 500.0f, 500.0f, 0.0f) };
	
};
