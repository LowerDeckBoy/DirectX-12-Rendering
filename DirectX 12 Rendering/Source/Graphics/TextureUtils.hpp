#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <string>

struct Descriptor;

struct TextureData
{
	uint64_t Width;
	uint32_t Height;
	uint16_t Depth;
	DXGI_FORMAT	Format{ DXGI_FORMAT_R8G8B8A8_UNORM };
	D3D12_RESOURCE_DIMENSION Dimension{ D3D12_RESOURCE_DIMENSION_TEXTURE2D };
};

struct TextureDesc
{
	D3D12_RESOURCE_FLAGS	Flag{ D3D12_RESOURCE_FLAG_NONE };
	D3D12_RESOURCE_STATES	State{ D3D12_RESOURCE_STATE_COMMON };
	CD3DX12_HEAP_PROPERTIES HeapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
	D3D12_HEAP_FLAGS		HeapFlag{ D3D12_HEAP_FLAG_NONE };
};

struct HeapProps
{
	inline static const CD3DX12_HEAP_PROPERTIES HeapDefault{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
	inline static const CD3DX12_HEAP_PROPERTIES HeapUpload { CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
};


class TextureUtils
{
public:
	// Resource creation
	static ID3D12Resource* CreateResource(ID3D12Device5* pDevice, TextureDesc Desc, TextureData Data);
	static void CreateSRV(ID3D12Device5* pDevice, ID3D12Resource* pResource, Descriptor& TargetDescriptor, DXGI_FORMAT Format, uint16_t Depth);
	static void CreateUAV(ID3D12Device5* pDevice, ID3D12Resource* pResource, Descriptor& TargetDescriptor, DXGI_FORMAT Format, uint16_t Depth);

	static ID3D12Resource* CreateFromWIC(DeviceContext* pDeviceContext, Descriptor& TargetDescriptor, const std::string_view& Filepath);
	//static ID3D12Resource* CreateFromDDS(DeviceContext* pDeviceContext, Descriptor& TargetDescriptor, const std::string_view& Filepath);

	// Uploaded Resource from HDR files
	static ID3D12Resource* CreateFromHDR(DeviceContext* pDeviceContext, const std::string_view& Filepath);

	//static void CreateFromResource();



private:


};

