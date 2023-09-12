#pragma once
#include "../Core/DeviceContext.hpp"


class ShadowMap
{
public:
	ShadowMap(DeviceContext* pDeviceCtx, ID3D12DescriptorHeap* pDepthHeap, uint32_t Width, uint32_t Height);
	~ShadowMap();

	void Create(DeviceContext* pDeviceCtx, ID3D12DescriptorHeap* pDepthHeap, uint32_t Width, uint32_t Height);
	
	[[maybe_unused]]
	void OnResize();

	void SetMapViewport(uint32_t Width, uint32_t Height);

	inline ID3D12Resource* GetResource() const { return m_ShadowMap.Get(); }
	const Descriptor& GetDescriptor() const { return m_Descriptor; }

	inline const D3D12_VIEWPORT& GetViewport() const { return m_Viewport; }
	inline const D3D12_RECT& GetScissor() const		 { return m_Scrissor; }

	void Release();

private:
	void CreateResource(DeviceContext* pDeviceCtx, ID3D12DescriptorHeap* pDepthHeap, uint32_t Width, uint32_t Height);
	void CreateDescriptors(DeviceContext* pDeviceCtx, ID3D12DescriptorHeap* pDepthHeap);

	ComPtr<ID3D12Resource> m_ShadowMap;
	Descriptor m_Descriptor;

	D3D12_VIEWPORT m_Viewport{};
	D3D12_RECT m_Scrissor{};

	DXGI_FORMAT m_Format{ DXGI_FORMAT_D32_FLOAT };

};

