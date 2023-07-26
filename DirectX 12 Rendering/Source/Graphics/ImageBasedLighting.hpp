#pragma once
#include "../Core/ComputePipelineState.hpp"
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
	void InitializeTextures(DeviceContext* pDevice, const std::string_view& Filepath);
	void InitializeBuffers(DeviceContext* pDevice);

	void Draw(Camera* pCamera, uint32_t FrameIndex);

	void Release();

	//ID3D12Resource* m_EnvironmentMap{ nullptr };
	Descriptor m_EnvDescriptor;
	//ID3D12Resource* m_IrradianceMap{ nullptr };
	//Descriptor m_IrradianceDescriptor;
	//ID3D12Resource* m_PrefilteredMap{ nullptr };
	//Descriptor m_PrefilteredDescriptor;

	Descriptor m_Descriptor;
	ComPtr<ID3D12Resource> m_OutputResource;
	Descriptor m_OutputDescriptor;

private:
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
