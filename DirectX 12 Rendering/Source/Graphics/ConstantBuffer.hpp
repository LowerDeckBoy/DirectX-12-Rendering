#pragma once
#include "../Core/DeviceContext.hpp"
#include "../Utilities/Utilities.hpp"
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

struct SceneConstData
{
	XMVECTOR Position;
	XMMATRIX View;
	XMMATRIX Projection;
	XMMATRIX InversedView;
	XMMATRIX InversedProjection;
	XMFLOAT2 ScreenDimension;
};

// Mainly for glTF model purposes
struct cbMaterial
{
	XMFLOAT4 CameraPosition	{ XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f) };

	XMFLOAT4 BaseColorFactor	{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0) };
	XMFLOAT4 EmissiveColorFactor{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0) };

	float MetallicFactor{ 1.0f };
	float RougnessFactor{ 1.0f };
	float AlphaCutoff	{ 0.5f };
	BOOL bDoubleSided	{ FALSE };

	//test
	int32_t BaseColorIndex{ -1 };
	int32_t NormalIndex{ -1 };
	int32_t MetallicRoughnessIndex{ -1 };
	int32_t EmissiveIndex{ -1 };

	//XMFLOAT4 padding[8]{};
};

struct cbLights
{
	std::array<XMFLOAT4, 4> LightPositions;
	std::array<XMFLOAT4, 4> LightColors;
	float Radius{ 25.0f };
	float padding[32];
};

struct cb_pbrMaterial
{
	XMFLOAT4 Ambient{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
	XMFLOAT4 Diffuse { XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f) };
	XMFLOAT4 Specular{ XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f) };
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
	const static uint32_t FRAME_COUNT{ 3 };
public:
	ConstantBuffer() = default;
	ConstantBuffer(DeviceContext* pDevice, T* pData) { Create(pDevice, pData); }
	~ConstantBuffer() { Release(); }

	void Create(DeviceContext* pDevice, T* pData)
	{
		if (bIsInitialized)
			return;

		constexpr size_t structSize{ (sizeof(T) + 255) & ~255 };

		auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
		auto bufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(structSize) };

		for (uint32_t i = 0; i < FRAME_COUNT; i++)
		{
			Data.at(i) = pData;

			D3D12MA::ALLOCATION_DESC allocDesc{};
			allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
			allocDesc.Flags = D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_COMMITTED | D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY;

			D3D12MA::Allocation* allocation{ nullptr };
			ThrowIfFailed(pDevice->GetAllocator()->CreateResource(&allocDesc, &bufferDesc, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, &allocation, IID_PPV_ARGS(Buffer.at(i).ReleaseAndGetAddressOf())));


			D3D12_CONSTANT_BUFFER_VIEW_DESC bufferView{};
			bufferView.BufferLocation = Buffer.at(i).Get()->GetGPUVirtualAddress();
			bufferView.SizeInBytes = static_cast<uint32_t>(structSize);

			const CD3DX12_RANGE readRange(0, 0);
			ThrowIfFailed(Buffer.at(i).Get()->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin.at(i))));
			std::memcpy(pDataBegin.at(i), &pData, sizeof(T));
			//Buffer.at(i).Get()->Unmap(0, nullptr);

			pDevice->GetMainHeap()->Allocate(m_Descriptors.at(i));
			pDevice->GetDevice()->CreateConstantBufferView(&bufferView, m_Descriptors.at(i).GetCPU());

			allocation->Release();
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

	std::array<ComPtr<ID3D12Resource>, FRAME_COUNT> Buffer;
	std::array<T*, FRAME_COUNT> Data{ nullptr, nullptr };
	std::array<Descriptor, FRAME_COUNT> m_Descriptors;

	bool bIsInitialized{ false };
};
