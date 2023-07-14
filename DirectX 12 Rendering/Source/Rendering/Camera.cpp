#include "Camera.hpp"
#include <imgui.h>


void Camera::Initialize(float AspectRatio) noexcept
{
	// Defaulting position on startup
	m_Position	= m_DefaultPosition;
	m_Target	= m_DefaultTarget;
	m_Up		= m_DefaultUp;

	m_View = XMMatrixLookAtLH(m_Position, m_Target, m_Up);
	m_Projection = XMMatrixPerspectiveFovLH(m_FoV, AspectRatio, m_zNear, m_zFar);
	
	m_CameraSlider = { XMVectorGetX(m_Position), XMVectorGetY(m_Position), XMVectorGetZ(m_Position) };
}

void Camera::Update() noexcept
{
	m_RotationMatrix = XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f);
	m_Target = XMVector3Normalize(XMVector3TransformCoord(m_DefaultForward, m_RotationMatrix));

	const XMMATRIX rotation{ XMMatrixRotationY(m_Yaw) };

	m_Forward = XMVector3TransformCoord(m_DefaultForward, rotation);
	m_Right = XMVector3TransformCoord(m_DefaultRight, rotation);
	m_Up = XMVector3TransformCoord(m_Up, rotation);

	m_Position += (MoveForwardBack * m_Forward);
	m_Position += (MoveRightLeft * m_Right);
	m_Position += (MoveUpDown * m_Upward);

	MoveForwardBack = 0.0f;
	MoveRightLeft	= 0.0f;
	MoveUpDown		= 0.0f;

	m_Target += m_Position;
	
	m_View = XMMatrixLookAtLH(m_Position, m_Target, m_Up);
	m_CameraSlider = { XMVectorGetX(m_Position), XMVectorGetY(m_Position), XMVectorGetZ(m_Position) };
}

void Camera::SetPosition(const DirectX::XMVECTOR NewPosition) noexcept
{
	m_Position = NewPosition;
}

void Camera::SetPosition(const std::array<float, 3> NewPosition) noexcept
{
	m_Position = XMVectorSet(NewPosition.at(0), NewPosition.at(1), NewPosition.at(2), 0.0f);
}

void Camera::ResetPitch() noexcept
{
	m_Pitch = 0.0;
}

void Camera::ResetYaw() noexcept
{
	m_Yaw = 0.0f;
}

void Camera::ResetCamera() noexcept
{
	m_Position = m_DefaultPosition;
	m_Target = m_DefaultTarget;
	m_Up = m_DefaultUp;
	m_Yaw = 0.0f;
	m_Pitch = 0.0f;
}

const XMMATRIX& Camera::GetView() const noexcept
{
	return m_View;
}

const XMMATRIX& Camera::GetProjection() const noexcept
{
	return m_Projection;
}

const XMMATRIX Camera::GetViewProjection() noexcept
{
	return XMMatrixMultiply(m_View, m_Projection);
}

const XMVECTOR& Camera::GetPosition() const noexcept
{
	return m_Position;
}

const XMFLOAT4 Camera::GetPositionFloat() const noexcept
{
	return XMFLOAT4(XMVectorGetX(m_Position), XMVectorGetY(m_Position), XMVectorGetZ(m_Position), 0.0f);
}

const XMVECTOR& Camera::GetTarget() const noexcept
{
	return m_Target;
}

const XMVECTOR& Camera::GetUp() const noexcept
{
	return m_Up;
}

void Camera::DrawGUI()
{
	//ImGui::Begin("Camera");

	if (ImGui::DragFloat3("Position", m_CameraSlider.data()))
	{
		SetPosition(m_CameraSlider);
		Update();
	}

	ImGui::SliderFloat("Speed", &m_CameraSpeed, 1.0f, 500.0f, "%.2f");

	if (ImGui::Button("Reset"))
	{
		ResetCamera();
		Update();
	}

	//ImGui::End();
}

void Camera::OnAspectRatioChange(float NewAspectRatio)
{
	m_Projection = XMMatrixPerspectiveFovLH(m_FoV, NewAspectRatio, m_zNear, m_zFar);
}

float Camera::GetCameraSpeed() const noexcept
{
	return m_CameraSpeed;
}

void Camera::SetCameraSpeed(float NewSpeed) noexcept
{
	m_CameraSpeed = NewSpeed;
}

void Camera::SetZNear(float NewZ) noexcept
{
	m_zNear = NewZ;
}

void Camera::SetZFar(float NewZ) noexcept
{
	m_zFar = NewZ;
}

inline float Camera::GetZNear() const noexcept
{
	return m_zNear;
}

inline float Camera::GetZFar() const noexcept
{
	return m_zFar;
}
