#include "Texture.hpp"
#include "../Rendering/Camera.hpp"
#include "Skybox.hpp"

void Skybox::Create(Device* pDevice)
{
	assert(m_Device = pDevice);

	m_VertexBuffer = std::make_unique<VertexBuffer<SkyboxVertex>>(pDevice, *m_Vertices);
	m_IndexBuffer = std::make_unique<IndexBuffer>(pDevice, *m_Indices);
	m_ConstBuffer = std::make_unique<ConstantBuffer<cbPerObject>>(pDevice, &m_cbData);

	//m_Texture = std::make_unique<Texture>(pDevice, "Assets/Textures/earth-cubemap.dds");
	//m_Texture = std::make_unique<Texture>(pDevice, "Assets/Textures/SunSubMixer_specularIBL.dds");
	//m_Texture = std::make_unique<Texture>(pDevice, "Assets/Textures/newport_loft.hdr");
	m_Texture = std::make_unique<Texture>(pDevice, "Assets/Textures/anime_art_style_magic_forest_with_intense_blue_lig.hdr");
	//m_Texture = std::make_unique<Texture>(pDevice, "Assets/Textures/HDR/anime_art_style_sunset_by_the_lake.hdr");
	//m_Texture = std::make_unique<Texture>(pDevice, "Assets/Textures/HDR/fantasy_lands_bright_nightsky_by_the_lake.hdr");
	//m_Texture = std::make_unique<Texture>(pDevice, "Assets/Textures/HDR/fantasy_lands_sunny_day_skymap_by_the_purple_star_.hdr");
	//m_Texture = std::make_unique<Texture>(pDevice, "Assets/Textures/HDR/fantasy_lands_sunset_by_the_lake.hdr");
	//m_Texture = std::make_unique<Texture>(pDevice, "Assets/Textures/HDR/fantasy_landscape_nightsky.hdr");
	//m_Texture = std::make_unique<Texture>(pDevice, "Assets/Textures/PaperMill_E_Env.hdr");
	//m_Texture = std::make_unique<Texture>(pDevice, "Assets/Textures/anime_art_style_sunny_day_sky.jpg");
	//m_Texture = std::make_unique<Texture>(pDevice, "Assets/Textures/fantasy_landscape_nightsky.jpeg");
	//m_Texture = std::make_unique<Texture>(pDevice, "Assets/Textures/fantasy_landscape_nightsky.hdr");
	//m_Texture->CreateDDS(pDevice, "Assets/Textures/SunSubMixer_diffuseIBL.dds");


}

void Skybox::Draw(Camera* pCamera)
{
	m_Device->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Device->GetCommandList()->IASetIndexBuffer(&m_IndexBuffer->GetBufferView());
	m_Device->GetCommandList()->IASetVertexBuffers(0, 1, &m_VertexBuffer->GetBufferView());

	UpdateWorld(pCamera);
	m_ConstBuffer->Update({ XMMatrixTranspose(m_WorldMatrix * pCamera->GetViewProjection()),
								XMMatrixTranspose(XMMatrixIdentity()) });
	m_Device->GetCommandList()->SetGraphicsRootConstantBufferView(0, m_ConstBuffer->GetBuffer()->GetGPUVirtualAddress());
	m_Device->GetCommandList()->SetGraphicsRootDescriptorTable(3, m_Texture->m_Descriptor.GetGPU());

	m_Device->GetCommandList()->DrawIndexedInstanced(m_IndexBuffer->GetSize(), 1, 0, 0, 0);
}

void Skybox::UpdateWorld(Camera* pCamera)
{
	m_WorldMatrix = XMMatrixIdentity();
	m_Translation = XMVectorSet(DirectX::XMVectorGetX(pCamera->GetPosition()),
								DirectX::XMVectorGetY(pCamera->GetPosition()),
								DirectX::XMVectorGetZ(pCamera->GetPosition()), 0.0f);
	m_WorldMatrix = XMMatrixScalingFromVector(m_Scale) * XMMatrixTranslationFromVector(m_Translation);
}

void Skybox::Release()
{
}
