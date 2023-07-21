#pragma once
#include "Importer.hpp"
#include "../../Graphics/Buffer.hpp"
#include "../../Graphics/ConstantBuffer.hpp"
#include <memory>
#include <array>

class DeviceContext;
class Camera;

class Model : public Importer
{
public:
	Model(DeviceContext* pDevice, std::string_view Filepath, const std::string& ModelName = "Non-given");
	~Model();

	void Create(DeviceContext* pDevice, std::string_view Filepath);
	void Draw(Camera* pCamera);

	void DrawGUI();

	void Release();

	std::unique_ptr<VertexBuffer> m_VertexBuffer;
	std::unique_ptr<IndexBuffer> m_IndexBuffer;

protected:
	// Constant Buffers
	std::unique_ptr<ConstantBuffer<cbPerObject>> m_cbPerObject;
	cbPerObject m_cbPerObjectData{};
	std::unique_ptr<ConstantBuffer<cbCamera>> m_cbCamera;
	cbCamera m_cbCameraData{};
	// Data for light shading
	std::unique_ptr<ConstantBuffer<cbMaterial>> m_cbMaterial;
	cbMaterial m_cbMaterialData{};

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
	std::array<float, 4> m_Diffuse	{ 1.0f, 1.0f, 1.0f, 0.0f };
	std::array<float, 4> m_Specular	{ 1.0f, 1.0f, 1.0f, 0.0f };
	float m_SpecularIntensity		{ 32.0f };
	std::array<float, 4> m_Direction{ 1.0f, 1.0f, 1.0f, 0.0f };

};
