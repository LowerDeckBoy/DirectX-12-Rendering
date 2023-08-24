#pragma once
#include "../Core/DeviceContext.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Core/DescriptorHeap.hpp"
#include <DirectXMath.h>

using namespace DirectX;
#include "ConstantTypes.hpp"

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

		//const auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
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

	std::array<uint8_t*, FRAME_COUNT> pDataBegin{ };

private:

	std::array<ComPtr<ID3D12Resource>, FRAME_COUNT> Buffer;
	std::array<T*, FRAME_COUNT> Data{ };
	std::array<Descriptor, FRAME_COUNT> m_Descriptors;

	bool bIsInitialized{ false };
};
