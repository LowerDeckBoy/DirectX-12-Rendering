#pragma once
#include "asssimp_Importer.hpp"
#include "../../Graphics/Buffer.hpp"
#include "../../Graphics/ConstantBuffer.hpp"
#include <memory>
#include <array>

class Camera;
class Device;

class assimp_Model : public asssimp_Importer
{
public:

	void Create(Device* pDevice, std::string_view Filepath);
	void Draw(Camera* pCamera);

	void DrawGUI();

private:

	std::unique_ptr<VertexBuffer<Vertex>> m_VertexBuffer;
	std::unique_ptr<IndexBuffer> m_IndexBuffer;
	//
	std::unique_ptr<ConstantBuffer<cbPerObject>> m_ConstBuffer;
	cbPerObject m_cbData{};
	std::unique_ptr<ConstantBuffer<cbCamera>> m_cbCamera;
	cbCamera m_cbCameraData{};
	// Data for light shading
	std::unique_ptr<ConstantBuffer<cbMaterial>> m_cbLight;
	cbMaterial m_cbLightData{};

	// Const buffer for light positions and colors -> PBR
	std::unique_ptr<ConstantBuffer<cbLights>> m_cbPointLights;
	cbLights m_cbPointLightsData{};


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
	std::array<XMFLOAT4, 4> m_LightPositions;
	std::array<std::array<float, 4>, 4> m_LightPositionsFloat;
	std::array<XMFLOAT4, 4> m_LightColors;
	std::array<std::array<float, 4>, 4> m_LightColorsFloat;
	void DoLights();
	void UpdateLights();
	void ResetLights();


};

