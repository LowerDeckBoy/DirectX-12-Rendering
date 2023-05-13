#pragma once

#include <array>
#include <DirectXMath.h>

class Camera
{
public:
	Camera operator=(const Camera&) = delete;

	void Initialize(float AspectRatio);
	void Update();

	void SetPosition(const DirectX::XMVECTOR NewPosition);
	void SetPosition(const std::array<float, 3> NewPosition);

	void ResetPitch();
	void ResetYaw();

	void ResetCamera();

	const inline DirectX::XMMATRIX& GetView() const { return m_View; }
	const inline DirectX::XMMATRIX& GetProjection() const { return m_Projection; }
	const inline DirectX::XMMATRIX GetViewProjection() { return XMMatrixMultiply(m_View, m_Projection); }

	const inline DirectX::XMVECTOR& GetPosition() const { return m_Position; }
	const inline DirectX::XMFLOAT3 GetPositionFloat() const { return DirectX::XMFLOAT3(m_Position.m128_f32[0], m_Position.m128_f32[1], m_Position.m128_f32[2]); }
	const inline DirectX::XMVECTOR& GetTarget() const { return m_Target; }
	const inline DirectX::XMVECTOR& GetUp() const { return m_Up; }

	void DrawGUI();

	// Required when window is resizing
	// thus Render Targets change their aspect ratio
	void OnAspectRatioChange(float NewAspectRatio);

private:
	DirectX::XMMATRIX m_View					{ DirectX::XMMATRIX() };
	DirectX::XMMATRIX m_Projection				{ DirectX::XMMATRIX() };
	DirectX::XMMATRIX m_ViewProjection			{ DirectX::XMMATRIX() };

	DirectX::XMVECTOR m_Position				{ DirectX::XMVECTOR() };
	DirectX::XMVECTOR m_Target					{ DirectX::XMVECTOR() };
	DirectX::XMVECTOR m_Up						{ DirectX::XMVECTOR() };

	DirectX::XMMATRIX m_RotationX				{ DirectX::XMMATRIX() };
	DirectX::XMMATRIX m_RotationY				{ DirectX::XMMATRIX() };
	DirectX::XMMATRIX m_RotationMatrix			{ DirectX::XMMATRIX() };

	DirectX::XMVECTOR m_Forward					{ DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f) };
	DirectX::XMVECTOR m_Right					{ DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f) };
	DirectX::XMVECTOR m_Upward					{ DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };

	DirectX::XMVECTOR const m_DefaultPosition	{ DirectX::XMVectorSet(0.0f, 5.0f, -10.0f, 0.0f) };
	DirectX::XMVECTOR const m_DefaultTarget		{ DirectX::XMVectorSet(0.0f, 5.0f, 0.0f, 0.0f) };
	DirectX::XMVECTOR const m_DefaultUp			{ DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };

	DirectX::XMVECTOR const m_DefaultForward	{ DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f) };
	DirectX::XMVECTOR const m_DefaultRight		{ DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f) };
	DirectX::XMVECTOR const m_DefaultUpward		{ DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };

	float m_zNear{ 0.1f };
	float m_zFar{ 50000.0f };

public:
	// For calling camera movement from keyboard inputs
	float MoveForwardBack{ 0.0f };
	float MoveRightLeft{ 0.0f };
	float MoveUpDown{ 0.0f };

	float m_Pitch{ 0.0f };
	float m_Yaw{ 0.0f };

	float GetCameraSpeed() const { return m_CameraSpeed; };
	void SetCameraSpeed(float NewSpeed) { m_CameraSpeed = NewSpeed; }

	float GetZNear() const    { return m_zNear; }
	void SetZNear(float NewZ) { m_zNear = NewZ; }
	float GetZFar() const     { return m_zFar; }
	void SetZFar(float NewZ)  { m_zFar = NewZ; }

	// For GUI usage
	std::array<float, 3> m_CameraSlider;

	inline static float m_CameraSpeed{ 25.0f };
private:

};
