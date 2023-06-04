#pragma once
#include "../Core/Device.hpp"
#include "../Utils/Utilities.hpp"
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

// Mainly for glTF model purposes
struct cbMaterial
{
	XMFLOAT4 Ambient		{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
	XMFLOAT4 Diffuse		{ XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f) };
	XMFLOAT4 Direction		{ XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f) };
	XMFLOAT4 CameraPosition	{ XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f) };

	XMFLOAT4 BaseColorFactor	{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0) };
	XMFLOAT4 EmissiveColorFactor{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0) };

	float MetallicFactor{ 1.0f };
	float RougnessFactor{ 1.0f };
	float AlphaCutoff	{ 0.5f };
	BOOL bDoubleSided	{ FALSE };

	BOOL bHasDiffuse	{ FALSE };
	BOOL bHasNormal		{ FALSE };
	BOOL bHasMetallic	{ FALSE };
	BOOL bHasEmissive	{ FALSE };
	
	XMFLOAT4 padding[8]{};
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
	const static uint32_t FRAME_COUNT{ 2 };
public:
	ConstantBuffer() = default;
	ConstantBuffer(Device* pDevice, T* pData) { Create(pDevice, pData); }
	~ConstantBuffer() { Release(); }

	void Create(Device* pDevice, T* pData)
	{
		if (bIsInitialized)
			return;

		size_t structSize{ (sizeof(T) + 255) & ~255 };

		auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
		auto bufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(structSize) };

		for (uint32_t i = 0; i < FRAME_COUNT; i++)
		{
			Data.at(i) = pData;

			ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr, IID_PPV_ARGS(Buffer[i].GetAddressOf())));
			// Debug name
			Buffer[i]->SetName(L"Model Constant Buffer");

			D3D12_CONSTANT_BUFFER_VIEW_DESC bufferView{};
			bufferView.BufferLocation = Buffer[i].Get()->GetGPUVirtualAddress();
			bufferView.SizeInBytes = static_cast<uint32_t>(sizeof((structSize)+255) & ~255);

			pDevice->GetMainHeap().Allocate(m_Descriptors.at(i));

			pDevice->GetDevice()->CreateConstantBufferView(&bufferView, m_Descriptors.at(i).GetCPU());

			CD3DX12_RANGE readRange(0, 0);
			ThrowIfFailed(Buffer[i].Get()->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin.at(i))));
			std::memcpy(pDataBegin.at(i), &pData, sizeof(T));
		}

		bIsInitialized = true;
	}

	void Update(const T& Updated, uint32_t Index)
	{
		*Data.at(Index) = Updated;
		std::memcpy(pDataBegin.at(Index), &Updated, sizeof(T));
	}

	void Release()
	{
		for (auto& buffer : Buffer)
			SAFE_RELEASE(buffer);
	}

	inline ID3D12Resource* GetBuffer(uint32_t Index)
	{
		if (Index >= 0 && Index <= FRAME_COUNT)
			return Buffer[Index].Get();

		return nullptr;
	}

	inline T* GetData(uint32_t Index)
	{
		if (Index >= 0 && Index <= FRAME_COUNT)
			return Data.at(Index);

		return nullptr;
	}

	Descriptor GetDescriptor(uint32_t Index) const
	{
		return m_Descriptors.at(Index);
	}

	std::array<uint8_t*, FRAME_COUNT> pDataBegin{ nullptr, nullptr };

private:

	Microsoft::WRL::ComPtr<ID3D12Resource> Buffer[FRAME_COUNT];
	std::array<T*, FRAME_COUNT> Data{ nullptr, nullptr };
	std::array<Descriptor, FRAME_COUNT> m_Descriptors;

	bool bIsInitialized{ false };
};

/*
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
*/
