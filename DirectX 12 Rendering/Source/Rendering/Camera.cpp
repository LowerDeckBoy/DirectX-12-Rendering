#include "Camera.hpp"
#include <imgui.h>

using namespace DirectX;

void Camera::Initialize(float AspectRatio)
{
	// Defaulting position on startup
	m_Position = m_DefaultPosition;
	m_Target = m_DefaultTarget;
	m_Up = m_DefaultUp;

	auto FoV{ 0.4f * XM_PI };
	m_View = XMMatrixLookAtLH(m_Position, m_Target, m_Up);
	m_Projection = XMMatrixPerspectiveFovLH(FoV, AspectRatio, m_zNear, m_zFar);
	m_ViewProjetion = XMMatrixMultiply(m_View, m_Projection);

	m_CameraSlider = { m_Position.m128_f32[0], m_Position.m128_f32[1], m_Position.m128_f32[2] };
}

void Camera::Update()
{
	m_RotationMatrix = XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f);
	m_Target = XMVector3Normalize(XMVector3TransformCoord(m_DefaultForward, m_RotationMatrix));
	
	XMMATRIX rotation{ XMMatrixRotationY(m_Yaw) };

	m_Forward = XMVector3TransformCoord(m_DefaultForward, rotation);
	m_Right = XMVector3TransformCoord(m_DefaultRight, rotation);
	m_Up = XMVector3TransformCoord(m_Up, rotation);

	m_Position += (MoveForwardBack * m_Forward);
	m_Position += (MoveRightLeft * m_Right);
	m_Position += (MoveUpDown * m_Upward);

	MoveForwardBack = 0.0f;
	MoveRightLeft = 0.0f;
	MoveUpDown = 0.0f;

	m_Target = m_Position + m_Target;
	
	m_View = XMMatrixLookAtLH(m_Position, m_Target, m_Up);
	m_ViewProjetion = XMMatrixMultiply(m_View, m_Projection);
	m_CameraSlider = { XMVectorGetX(m_Position), XMVectorGetY(m_Position), XMVectorGetZ(m_Position) };
}

void Camera::SetPosition(const DirectX::XMVECTOR NewPosition)
{
	m_Position = NewPosition;
}

void Camera::SetPosition(const std::array<float, 3> NewPosition)
{
	m_Position = XMVectorSet(NewPosition.at(0), NewPosition.at(1), NewPosition.at(2), 0.0f);
}

void Camera::ResetPitch()
{
	m_Pitch = 0.0;
}

void Camera::ResetYaw()
{
	m_Yaw = 0.0f;
}

void Camera::ResetCamera()
{
	m_Position = m_DefaultPosition;
	m_Target = m_DefaultTarget;
	m_Up = m_DefaultUp;
	m_Yaw = 0.0f;
	m_Pitch = 0.0f;
}

void Camera::DrawGUI()
{
	ImGui::Begin("Camera");

	if (ImGui::DragFloat3("Position", m_CameraSlider.data()))
	{
		SetPosition(m_CameraSlider);
		Update();
	}
	if (ImGui::Button("Reset"))
	{
		ResetCamera();
		Update();
	}

	ImGui::End();
}

void Camera::OnAspectRatioChange(float NewAspectRatio)
{
	auto FoV{ 0.4f * XM_PI };
	m_Projection = XMMatrixPerspectiveFovLH(FoV, NewAspectRatio, m_zNear, m_zFar);
}
