#pragma once
//#include "Vertex.hpp"
#include "../Utils/Utils.hpp"
//#include "../Core/Device.hpp"

template<typename T>
class VertexBuffer
{
public:
	void Create(Device* pDevice, std::vector<T>& pData)
	{
		size_t bufferSize{ sizeof(T) * pData.size() };
		auto heapDesc{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize) };

		auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
		ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(&heapProperties,
						D3D12_HEAP_FLAG_NONE, &heapDesc,
						D3D12_RESOURCE_STATE_GENERIC_READ,
						nullptr,
						IID_PPV_ARGS(Buffer.GetAddressOf())));
		
		uint8_t* pDataBegin{};
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(Buffer.Get()->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
		std::memcpy(pDataBegin, pData.data(), bufferSize);
		Buffer.Get()->Unmap(0, nullptr);

		BufferView.BufferLocation	= Buffer.Get()->GetGPUVirtualAddress();
		BufferView.SizeInBytes		= static_cast<uint32_t>(bufferSize);
		BufferView.StrideInBytes	= static_cast<uint32_t>(sizeof(T));

	}

	[[nodiscard]]
	inline ID3D12Resource* GetBuffer() const { return Buffer.Get(); }

	[[nodiscard]]
	inline D3D12_VERTEX_BUFFER_VIEW& GetBufferView() { return BufferView; }


private:
	Microsoft::WRL::ComPtr<ID3D12Resource> BufferUploadHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> Buffer;
	D3D12_VERTEX_BUFFER_VIEW BufferView{};

};

class IndexBuffer
{
public:

	void Create(Device* pDevice, std::vector<uint32_t>& pData)
	{
		// indices count
		BufferSize = static_cast<uint32_t>(pData.size());

		size_t bufferSize = sizeof(uint32_t) * pData.size();
		auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)};
		auto heapDesc{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize) };
		ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(&heapProperties,
					  D3D12_HEAP_FLAG_NONE,
					  &heapDesc,
					  D3D12_RESOURCE_STATE_GENERIC_READ,
					  nullptr,
					  IID_PPV_ARGS(&Buffer)));

		uint8_t* pDataBegin{};
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(Buffer.Get()->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
		memcpy(pDataBegin, pData.data(), bufferSize);
		Buffer.Get()->Unmap(0, nullptr);

		BufferView.BufferLocation = Buffer->GetGPUVirtualAddress();
		BufferView.Format = DXGI_FORMAT_R32_UINT;
		BufferView.SizeInBytes = static_cast<uint32_t>(bufferSize);
	}


	[[nodiscard]]
	inline ID3D12Resource* GetBuffer() const { return Buffer.Get(); }

	[[nodiscard]]
	inline D3D12_INDEX_BUFFER_VIEW& GetBufferView() { return BufferView; }

	[[nodiscard]]
	inline uint32_t GetSize() const { return BufferSize; }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> Buffer;
	D3D12_INDEX_BUFFER_VIEW BufferView;
	uint32_t BufferSize{};
};