#pragma once

#include <DirectXMath.h>
#include <array>

class Camera
{
public:
	
	void Initialize(float AspectRatio);
	void Update();

	void SetPosition(const DirectX::XMVECTOR NewPosition);
	void SetPosition(const std::array<float, 3> NewPosition);

	void ResetPitch();
	void ResetYaw();

	void ResetCamera();

	const DirectX::XMMATRIX& GetView() const { return m_View; }
	const DirectX::XMMATRIX& GetProjection() const { return m_Projection; }
	const DirectX::XMMATRIX& GetViewProjection() const { return m_ViewProjetion; }

	const DirectX::XMVECTOR& GetPosition() const { return m_Position; }
	const DirectX::XMVECTOR& GetTarget() const { return m_Target; }
	const DirectX::XMVECTOR& GetUp() const { return m_Up; }

	void DrawGUI();

	// Required when window is resizing
	// thus Render Targets change their aspect ratio
	void OnAspectRatioChange(float NewAspectRatio);

private:
	DirectX::XMMATRIX m_View{ DirectX::XMMATRIX() };
	DirectX::XMMATRIX m_Projection{ DirectX::XMMATRIX() };
	DirectX::XMMATRIX m_ViewProjetion{ DirectX::XMMATRIX() };

	DirectX::XMVECTOR m_Position{ DirectX::XMVectorSet(0.0f, 10.0f, -25.0f, 0.0f) };
	DirectX::XMVECTOR m_Target{ DirectX::XMVectorSet(0.0f, 10.0f, 0.0f, 0.0f) };
	DirectX::XMVECTOR m_Up{ DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };

	DirectX::XMMATRIX m_RotationX{ DirectX::XMMATRIX() };
	DirectX::XMMATRIX m_RotationY{ DirectX::XMMATRIX() };
	DirectX::XMMATRIX m_RotationMatrix{ DirectX::XMMATRIX() };

	DirectX::XMVECTOR m_Forward{ DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f) };
	DirectX::XMVECTOR m_Right{ DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f) };
	DirectX::XMVECTOR m_Upward{ DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };

	DirectX::XMVECTOR const m_DefaultPosition{ DirectX::XMVectorSet(0.0f, 5.0f, -10.0f, 0.0f) };
	DirectX::XMVECTOR const m_DefaultTarget{ DirectX::XMVectorSet(0.0f, 5.0f, 0.0f, 0.0f) };
	DirectX::XMVECTOR const m_DefaultUp{ DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };

	DirectX::XMVECTOR const m_DefaultForward{ DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f) };
	DirectX::XMVECTOR const m_DefaultRight{ DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f) };
	DirectX::XMVECTOR const m_DefaultUpward{ DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };

	float m_zNear{ 0.1f };
	float m_zFar{ 5000.0f };

public:
	float MoveForwardBack{};
	float MoveRightLeft{};
	float MoveUpDown{};

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

private:
	float m_CameraSpeed{ 25.0f };

};
