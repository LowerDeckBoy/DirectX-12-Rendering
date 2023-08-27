#pragma once
#include "Buffer/Buffer.hpp"
#include "Buffer/ConstantBuffer.hpp"

class DeviceContext;
class Camera;
class Descriptor;

class ImageBasedLighting
{
public:
	ImageBasedLighting() noexcept {}
	~ImageBasedLighting();

	void Create(DeviceContext* pDeviceCtx, const std::string_view& Filepath);
	
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
	void CreateTextures(DeviceContext* pDeviceCtx, const std::string_view& Filepath);
	void CreateBuffers(DeviceContext* pDeviceCtx);

	// Convert equirectangular HDR texture into TextureCube
	void CreateCubeTexture(DeviceContext* pDeviceCtx, ID3D12RootSignature* pComputeRoot, const std::string_view& Filepath);
	// Create 32x32 Irradiance TextureCube
	void CreateIrradiance(DeviceContext* pDeviceCtx, ID3D12RootSignature* pComputeRoot);
	// https://learnopengl.com/PBR/IBL/Specular-IBL
	// 256x256 specular reflection map
	void CreateSpecular(DeviceContext* pDeviceCtx, ID3D12RootSignature* pComputeRoot);
	void CreateSpecularBRDF(DeviceContext* pDeviceCtx, ID3D12RootSignature* pComputeRoot);

	ComPtr<ID3D12GraphicsCommandList4> m_CommandList;

	VertexBuffer m_VertexBuffer;
	IndexBuffer m_IndexBuffer;
	ConstantBuffer<cbPerObject> m_ConstBuffer;
	cbPerObject m_cbData{};

	void UpdateWorld(Camera* pCamera);

	XMMATRIX m_WorldMatrix	{ XMMatrixIdentity() };
	XMVECTOR m_Translation	{ XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
	XMVECTOR m_Scale		{ XMVectorSet(500.0f, 500.0f, 500.0f, 0.0f) };
	
};
