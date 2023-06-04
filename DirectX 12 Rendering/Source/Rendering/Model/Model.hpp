#pragma once
#include "Importer.hpp"
#include "../../Graphics/Buffer.hpp"
#include "../../Graphics/ConstantBuffer.hpp"
#include <memory>
#include <array>

class Camera;
class Device;
class Skybox;

class Model : public Importer
{
public:
	Model(Device* pDevice, std::string_view Filepath);
	~Model();

	void Create(Device* pDevice, std::string_view Filepath);
	void Draw(Camera* pCamera);

	void DrawGUI();

	void Release();

	void SetSkyTexture(Skybox* pTexture)
	{
		m_SkyTexture = pTexture;
		//assert(m_SkyTexture != nullptr);
	}

private:
	std::unique_ptr<VertexBuffer> m_VertexBuffer;
	std::unique_ptr<IndexBuffer> m_IndexBuffer;
	//
	std::unique_ptr<ConstantBuffer<cbPerObject>> m_ConstBuffer;
	cbPerObject m_cbData{};
	std::unique_ptr<ConstantBuffer<cbCamera>> m_cbCamera;
	cbCamera m_cbCameraData{};
	// Data for light shading
	std::unique_ptr<ConstantBuffer<cbMaterial>> m_cbMaterial;
	cbMaterial m_cbMaterialData{};

	// Const buffer for light positions and colors -> PBR
	std::unique_ptr<ConstantBuffer<cbLights>> m_cbPointLights;
	cbLights m_cbPointLightsData{};

	//TEST
	// reference to skybox
	Skybox* m_SkyTexture;

protected:
	// Transforms
	void UpdateWorld();
	XMMATRIX m_WorldMatrix	{ XMMatrixIdentity() };
	XMVECTOR m_Translation	{ XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
	XMVECTOR m_Rotation		{ XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f) };
	XMVECTOR m_Scale		{ XMVectorSet(1.0f, 1.0f, 1.0f, 0.0) };
	// For GUI usage
	std::array<float, 3> m_Translations	{ 0.0f, 0.0f, 0.0f };
	std::array<float, 3> m_Rotations	{ 0.0f, 0.0f, 0.0f };
	std::array<float, 3> m_Scales		{ 1.0f, 1.0f, 1.0f };

	// Shader data
	std::array<float, 4> m_Ambient	{ 1.0f, 1.0f, 1.0f, 1.0f };
	std::array<float, 3> m_Diffuse	{ 1.0f, 1.0f, 1.0f };
	std::array<float, 3> m_Specular	{ 1.0f, 1.0f, 1.0f };
	float m_SpecularIntensity		{ 32.0f };
	std::array<float, 3> m_Direction{ 1.0f, 1.0f, 1.0f };

	// TEST 
	//Light positions
	std::array<XMFLOAT4, 4>				m_LightPositions;
	std::array<std::array<float, 4>, 4> m_LightPositionsFloat;
	std::array<XMFLOAT4, 4>				m_LightColors;
	std::array<std::array<float, 4>, 4> m_LightColorsFloat;
	void DoLights();
	void UpdateLights();
	void ResetLights();

};
