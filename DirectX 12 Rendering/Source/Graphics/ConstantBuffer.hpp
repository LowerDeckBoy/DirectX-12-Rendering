#pragma once
#include "../Utils/Utils.hpp"
#include "../Core/DescriptorHeap.hpp"
#include <DirectXMath.h>

struct cbPerObject
{
	DirectX::XMMATRIX WVP	{ DirectX::XMMatrixIdentity() };
	DirectX::XMMATRIX World	{ DirectX::XMMatrixIdentity() };
	float padding[32]{};
};

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

		pDevice->GetDevice()->CreateConstantBufferView(&bufferView, m_Descriptor.m_cpuHandle);

		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(Buffer.Get()->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
		std::memcpy(pDataBegin, &pData, sizeof(T));

		bIsInitialized = true;
	}

	void Update(T& Updated)
	{
		Data = Updated;
		std::memcpy(pDataBegin, &Updated, sizeof(T));
	}

	void Release()
	{
		SafeRelease(Buffer);
	}

	[[nodiscard]]
	inline ID3D12Resource* GetBuffer() { return Buffer.Get(); }
	ID3D12Resource* GetBuffer() const { return Buffer.Get(); }
	Descriptor GetDescriptor() const { return m_Descriptor; }
	T* GetData() const { return Data; }
	uint8_t* pDataBegin{ nullptr };

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> Buffer;
	T* Data{ nullptr };
	Descriptor m_Descriptor;

	bool bIsInitialized{ false };
};
