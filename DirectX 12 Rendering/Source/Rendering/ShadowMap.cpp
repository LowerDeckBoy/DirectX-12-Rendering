#include "ShadowMap.hpp"
#include "../Utilities/Utilities.hpp"

ShadowMap::ShadowMap(DeviceContext* pDeviceCtx, ID3D12DescriptorHeap* pDepthHeap, uint32_t Width, uint32_t Height)
{
	Create(pDeviceCtx, pDepthHeap, Width, Height);
}

ShadowMap::~ShadowMap()
{
	Release();
}

void ShadowMap::Create(DeviceContext* pDeviceCtx, ID3D12DescriptorHeap* pDepthHeap, uint32_t Width, uint32_t Height)
{
	CreateResource(pDeviceCtx, pDepthHeap, Width, Height);

	CreateDescriptors(pDeviceCtx, pDepthHeap);

	SetMapViewport(Width, Height);
}

void ShadowMap::OnResize()
{
}

void ShadowMap::SetMapViewport(uint32_t Width, uint32_t Height)
{
	m_Scrissor.left		= 0L;
	m_Scrissor.top		= 0L;
	m_Scrissor.right	= static_cast<uint64_t>(Width);
	m_Scrissor.bottom	= static_cast<uint64_t>(Height);

	m_Viewport.TopLeftX = 0.0f;
	m_Viewport.TopLeftY = 0.0f;
	m_Viewport.Width	= static_cast<float>(m_Scrissor.right);
	m_Viewport.Height	= static_cast<float>(m_Scrissor.bottom);
	m_Viewport.MinDepth = D3D12_MIN_DEPTH;
	m_Viewport.MaxDepth = D3D12_MAX_DEPTH;
}

void ShadowMap::Release()
{
	SAFE_RELEASE(m_ShadowMap);

}

void ShadowMap::CreateResource(DeviceContext* pDeviceCtx, ID3D12DescriptorHeap* pDepthHeap, uint32_t Width, uint32_t Height)
{
	D3D12_RESOURCE_DESC desc{};
	desc.Width = static_cast<uint64_t>(Width);
	desc.Height = Height;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	desc.SampleDesc = { 1, 0 };

	if (m_Format == DXGI_FORMAT_D32_FLOAT)
		desc.Format = DXGI_FORMAT_R32_TYPELESS;
	else if (m_Format == DXGI_FORMAT_D24_UNORM_S8_UINT)
		desc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = m_Format;
	clearValue.DepthStencil.Depth = D3D12_MAX_DEPTH;
	clearValue.DepthStencil.Stencil = 0;

	const auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };

	ThrowIfFailed(pDeviceCtx->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&clearValue,
		IID_PPV_ARGS(m_ShadowMap.ReleaseAndGetAddressOf())));
	m_ShadowMap.Get()->SetName(L"Shadow Map Resource");

}

void ShadowMap::CreateDescriptors(DeviceContext* pDeviceCtx, ID3D12DescriptorHeap* pDepthHeap)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC dsView{};
	dsView.Flags = D3D12_DSV_FLAG_NONE;
	dsView.Format = DXGI_FORMAT_D32_FLOAT;
	dsView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsView.Texture2D.MipSlice = 0;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	pDeviceCtx->GetDevice()->CreateDepthStencilView(m_ShadowMap.Get(), &dsView, pDepthHeap->GetCPUDescriptorHandleForHeapStart());

	pDeviceCtx->GetMainHeap()->Allocate(m_Descriptor);
	pDeviceCtx->GetDevice()->CreateShaderResourceView(m_ShadowMap.Get(), &srvDesc, m_Descriptor.GetCPU());
}
