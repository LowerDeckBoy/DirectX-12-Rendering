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
	ImageBasedLighting() noexcept {}
	~ImageBasedLighting();

	void Create(DeviceContext* pDevice, const std::string_view& Filepath);
	
	void Draw(Camera* pCamera, uint32_t FrameIndex);

	void Release();
	
	ComPtr<ID3D12Resource> m_IrradianceMap;
	Descriptor m_IrradianceDescriptor;

	ComPtr<ID3D12Resource> m_SpecularMap;
	Descriptor m_SpecularDescriptor;

	ComPtr<ID3D12Resource> m_SpecularBRDF_LUT;
	Descriptor m_SpBRDFDescriptor;

	ComPtr<ID3D12Resource> m_OutputResource;
	Descriptor m_OutputDescriptor;

private:
	void CreateTextures(DeviceContext* pDevice, const std::string_view& Filepath);
	void CreateBuffers(DeviceContext* pDevice);

	// Convert equirectangular HDR texture into TextureCube
	void CreateCubeTexture(DeviceContext* pDeviceCtx, ID3D12RootSignature* pComputeRoot, const std::string_view& Filepath);
	// Create 32x32 Irradiance TextureCube
	void CreateIrradiance(DeviceContext* pDeviceCtx, ID3D12RootSignature* pComputeRoot);
	// 
	void CreateSpecular(DeviceContext* pDeviceCtx, ID3D12RootSignature* pComputeRoot);
	void CreateSpecularBRDF(DeviceContext* pDeviceCtx, ID3D12RootSignature* pComputeRoot);

	ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	VertexBuffer m_VertexBuffer;
	IndexBuffer m_IndexBuffer;
	ConstantBuffer<cbPerObject> m_ConstBuffer;
	cbPerObject m_cbData{};

	void UpdateWorld(Camera* pCamera);

	XMMATRIX m_WorldMatrix{ XMMatrixIdentity() };
	XMVECTOR m_Translation{ XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
	XMVECTOR m_Scale{ XMVectorSet(500.0f, 500.0f, 500.0f, 0.0f) };
	
};
