#pragma once
//#include <d3d12.h>
//#include <DirectXMath.h>
//#include <wrl.h>

#include "../Utils/Utils.hpp"
//#include "../Core/Device.hpp"

//class Device;

template<typename T>
class ConstantBuffer
{
public:
	~ConstantBuffer()
	{
		Release();
	}

	void Create(Device* pDevice, T* pData)
	{
		if (bIsInitialized)
			return;

		Data = pData;
		auto structSize{ sizeof(T) };

		auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
		auto heapDesc{ CD3DX12_RESOURCE_DESC::Buffer(structSize) };

		ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(&heapProperties,
					  D3D12_HEAP_FLAG_NONE,
					  &heapDesc,
					  D3D12_RESOURCE_STATE_GENERIC_READ,
					  nullptr, IID_PPV_ARGS(Buffer.GetAddressOf())));

		D3D12_CONSTANT_BUFFER_VIEW_DESC bufferView{};
		bufferView.BufferLocation = Buffer.Get()->GetGPUVirtualAddress();
		bufferView.SizeInBytes = static_cast<uint32_t>(structSize);

		pDevice->GetDevice()->CreateConstantBufferView(&bufferView, pDevice->GetCBVHeap()->GetCPUDescriptorHandleForHeapStart());

		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(Buffer.Get()->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
		std::memcpy(pDataBegin, &pData, sizeof(pData));
		//Buffer.Get()->Unmap(0, nullptr);

		bIsInitialized = true;
	}

	void Release()
	{
		//delete pDataBegin;
		//Buffer.Get()->Unmap(0, nullptr);
		SafeRelease(Buffer);
	}

	[[nodiscard]]
	inline ID3D12Resource* GetBuffer() { return Buffer.Get(); }

	uint8_t* pDataBegin{ nullptr };
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> Buffer;
	T* Data{ nullptr };
	//D3D12_CONSTANT_BUFFER_VIEW_DESC BufferView{};

	bool bIsInitialized{ false };
};

