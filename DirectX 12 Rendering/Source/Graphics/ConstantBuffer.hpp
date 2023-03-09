#pragma once
//#include <d3d12.h>
#include <DirectXMath.h>
//#include <wrl.h>

#include "../Utils/Utils.hpp"
//#include "../Core/Device.hpp"
#include "../Core/DescriptorHeap.hpp"
//class Device;

struct cbPerObject
{
	DirectX::XMMATRIX WVP{ DirectX::XMMatrixIdentity() };
	DirectX::XMMATRIX World{ DirectX::XMMatrixIdentity() };
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
		auto structSize{ sizeof(T) };

		//auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
		auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
		//auto heapDesc{ CD3DX12_RESOURCE_DESC::Buffer(structSize) };

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Width = structSize;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		//D3D12_RESOURCE_STATE_GENERIC_READ
		ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(&heapProperties,
					  D3D12_HEAP_FLAG_NONE,
					  &desc,
					  D3D12_RESOURCE_STATE_COMMON,
					  nullptr, IID_PPV_ARGS(Buffer.GetAddressOf())));
		// Debug name
		Buffer->SetName(L"Model Constant Buffer");

		D3D12_CONSTANT_BUFFER_VIEW_DESC bufferView{};
		bufferView.BufferLocation = Buffer.Get()->GetGPUVirtualAddress();
		bufferView.SizeInBytes = (sizeof(static_cast<uint32_t>(structSize) + 255)) & ~255;

		pDevice->m_cbvDescriptorHeap.Allocate(m_Descriptor);
		
		//pDevice->m_cbvDescriptorHeaps.at(0).Allocate(m_Descriptor);
		//pDevice->m_cbvDescriptorHeaps.at(1).Allocate(m_Descriptor);

		pDevice->GetDevice()->CreateConstantBufferView(&bufferView, m_Descriptor.m_cpuHandle);

		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(Buffer.Get()->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
		std::memcpy(pDataBegin, &pData, sizeof(pData));

		bIsInitialized = true;
	}

	void Update(T& Updated)
	{
		//Data = Updated;
		//std::memcpy(pDataBegin, &Data, sizeof(Data));
		std::memcpy(pDataBegin, &Updated, sizeof(Updated));
		
	}

	void Release()
	{
		//delete pDataBegin;
		//Buffer.Get()->Unmap(0, nullptr);
		SafeRelease(Buffer);
	}

	[[nodiscard]]
	inline ID3D12Resource* GetBuffer() { return Buffer.Get(); }

	ID3D12Resource* GetBuffer() const { return Buffer.Get(); }
	Descriptor GetDescriptor() const { return m_Descriptor; }
	T* GetData() const { return Data; }
	uint8_t* pDataBegin{ nullptr };

	//CD3DX12_CPU_DESCRIPTOR_HANDLE m_cpuDescriptor{};
	//CD3DX12_GPU_DESCRIPTOR_HANDLE m_gpuDescriptor{};

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> Buffer;
	T* Data{ nullptr };
	Descriptor m_Descriptor;
	//D3D12_CONSTANT_BUFFER_VIEW_DESC BufferView{};

	bool bIsInitialized{ false };
};
