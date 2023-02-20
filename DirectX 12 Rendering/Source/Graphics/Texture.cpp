#include "../Core/Device.hpp"
#include "Texture.hpp"
#include "../Utils/Utils.hpp"

Texture::Texture()
{
}

Texture::Texture(Device* pDevice, const std::string& TexturePath)
{
	assert(m_Device = pDevice);
}

Texture::~Texture()
{
}

void Texture::Initialize(Device* pDevice, const std::string& TexturePath)
{
	assert(m_Device = pDevice);

	std::wstring wpath = std::wstring(TexturePath.begin(), TexturePath.end());
	const wchar_t* path = wpath.c_str();

	DirectX::ScratchImage* scratchImage = new DirectX::ScratchImage();
	DirectX::LoadFromWICFile(path,
							 DirectX::WIC_FLAGS_FORCE_RGB,
							 nullptr,
							 *scratchImage);

	DirectX::TexMetadata metadata{ scratchImage->GetMetadata() };

	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Format = metadata.format;
	desc.Width =  static_cast<uint32_t>(metadata.width);
	desc.Height = static_cast<uint32_t>(metadata.height);
	desc.MipLevels = 1;
	desc.DepthOrArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	D3D12_SUBRESOURCE_DATA subresource{
		scratchImage->GetPixels(),
		static_cast<LONG_PTR>(scratchImage->GetImages()->rowPitch),
		static_cast<LONG_PTR>(scratchImage->GetImages()->slicePitch)
	};

	//Microsoft::WRL::ComPtr<ID3D12Resource> texture;

	auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
	ThrowIfFailed(m_Device->GetDevice()->CreateCommittedResource(&heapProperties,
				  D3D12_HEAP_FLAG_NONE,
				  &desc,
				  D3D12_RESOURCE_STATE_COPY_DEST,
				  nullptr,
				  IID_PPV_ARGS(&m_Texture)));
	//.GetAddressOf()
	const uint64_t bufferSize{ GetRequiredIntermediateSize(m_Texture.Get(), 0, 1) };

	heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	const auto bufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize) };
	ThrowIfFailed(pDevice->GetDevice()->CreateCommittedResource(&heapProperties,
				  D3D12_HEAP_FLAG_NONE,
				  &bufferDesc,
				  D3D12_RESOURCE_STATE_GENERIC_READ,
				  nullptr,
				  IID_PPV_ARGS(m_TextureUploadHeap.GetAddressOf())));
	m_TextureUploadHeap->SetName(L"Temporal texture");

	UpdateSubresources(pDevice->GetCommandList(), m_Texture.Get(), m_TextureUploadHeap.Get(), 0, 0, 1, &subresource);

	const auto copyToResourceBarrier{ CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(),
																	 D3D12_RESOURCE_STATE_COPY_DEST,
																	 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) };
	m_Device->GetCommandList()->ResourceBarrier(1, &copyToResourceBarrier);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	const auto cpuHandle{ m_Device->m_srvHeap->GetCPUDescriptorHandleForHeapStart() };
	m_Device->GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, cpuHandle);

	//ThrowIfFailed(m_Device->GetCommandList()->Close());
	//ID3D12CommandList* ppCommandLists[] = { m_Device->GetCommandList() };
	//m_Device->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists); 

	//delete scratchImage;
}

void Texture::Release()
{
}
