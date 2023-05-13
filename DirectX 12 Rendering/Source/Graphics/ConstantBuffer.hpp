#pragma once
#include "../Utils/Utils.hpp"
#include "../Core/DescriptorHeap.hpp"
#include <DirectXMath.h>
using namespace DirectX;

// Const Types
struct cbPerObject
{
	XMMATRIX WVP	{ XMMatrixIdentity() };
	XMMATRIX World	{ XMMatrixIdentity() };
	float padding[32]{};
};

struct cbCamera
{
	alignas(16) XMFLOAT3 CameraPosition;
};

struct cbMaterial
{
	XMFLOAT4 Ambient				{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
	alignas(16) XMFLOAT3 Diffuse	{ XMFLOAT3(1.0f, 1.0f, 1.0f) };
	alignas(16) XMFLOAT3 Direction	{ XMFLOAT3(1.0f, 1.0f, 1.0f) };
	alignas(16) XMFLOAT3 CameraPosition	{ XMFLOAT3(1.0f, 1.0f, 1.0f) };
	XMFLOAT4 padding[13];
};

struct cbLights
{
	std::array<XMFLOAT4, 4> LightPositions;
	std::array<XMFLOAT4, 4> LightColors;
	float padding[32];
};

struct cb_pbrMaterial
{
	alignas(16) XMFLOAT4 Ambient{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
	alignas(16) XMFLOAT3 Diffuse { XMFLOAT3(1.0f, 1.0f, 1.0f) };
	XMFLOAT3 Specular{ XMFLOAT3(1.0f, 1.0f, 1.0f) };
	float SpecularIntensity{ 32.0f };
	alignas(16) XMFLOAT3 Direction { XMFLOAT3(1.0f, 1.0f, 1.0f) };

	XMFLOAT4 BaseColorFactor{ XMFLOAT4(1.0f, 1.0f,1.0f, 1.0f) };
	XMFLOAT4 EmissiveFactor { XMFLOAT4(1.0f, 1.0f,1.0f, 1.0f) };

	float MetallicFactor { 1.0f };
	float RoughnessFactor{ 1.0f };
	float AlphaCutoff	 { 0.5f };
	float padding;

	XMFLOAT4 padding2[9];
};

template<typename T>
class ConstantBuffer
{
public:
	ConstantBuffer() = default;
	ConstantBuffer(Device* pDevice, T* pData) { Create(pDevice, pData); }
	~ConstantBuffer() { Release(); }
	
	void Create(Device* pDevice, T* pData)
	{
		if (bIsInitialized)
			return;

		Data = pData;
		size_t structSize{ (sizeof(T) + 255) & ~255 };

		auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
		auto bufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(structSize) };
		ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(&heapProperties,
					  D3D12_HEAP_FLAG_NONE,
					  &bufferDesc,
					  D3D12_RESOURCE_STATE_COMMON,
					  nullptr, IID_PPV_ARGS(Buffer.GetAddressOf())));
		// Debug name
		Buffer->SetName(L"Model Constant Buffer");

		D3D12_CONSTANT_BUFFER_VIEW_DESC bufferView{};
		bufferView.BufferLocation = Buffer.Get()->GetGPUVirtualAddress();
		bufferView.SizeInBytes = static_cast<uint32_t>(sizeof((structSize) + 255) & ~255);
		
		pDevice->m_cbvDescriptorHeap.Allocate(m_Descriptor);

		pDevice->GetDevice()->CreateConstantBufferView(&bufferView, m_Descriptor.GetCPU());

		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(Buffer.Get()->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
		std::memcpy(pDataBegin, &pData, sizeof(T));

		bIsInitialized = true;
	}

	void Update(const T& Updated)
	{
		*Data = Updated;
		std::memcpy(pDataBegin, &Updated, sizeof(T));
	}

	void Release()
	{
		SafeRelease(Buffer);
	}

	[[nodiscard]]
	inline ID3D12Resource* GetBuffer() { return Buffer.Get(); }
	Descriptor GetDescriptor() const { return m_Descriptor; }
	T* GetData() const { return Data; }
	uint8_t* pDataBegin{ nullptr };

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> Buffer;
	T* Data{ nullptr };
	Descriptor m_Descriptor;

	bool bIsInitialized{ false };
};
