#pragma once
#include <array>
#include <DirectXMath.h>

using namespace DirectX;
class Camera
{
public:
	Camera operator=(const Camera&) = delete;

	void Initialize(float AspectRatio) noexcept;
	void Update() noexcept;

	void SetPosition(const DirectX::XMVECTOR NewPosition) noexcept;
	void SetPosition(const std::array<float, 3> NewPosition) noexcept;

	void ResetPitch() noexcept;
	void ResetYaw() noexcept;

	void ResetCamera() noexcept;

	const XMMATRIX& GetView() const noexcept;
	const XMMATRIX& GetProjection() const noexcept;
	const XMMATRIX GetViewProjection() noexcept;

	const XMVECTOR& GetPosition() const noexcept;
	const XMFLOAT4 GetPositionFloat() const noexcept;
	const XMVECTOR& GetTarget() const noexcept;
	const XMVECTOR& GetUp() const noexcept;

	void DrawGUI();

	// Required when window is resizing
	// thus Render Targets change their aspect ratio
	void OnAspectRatioChange(float NewAspectRatio);

private:
	XMMATRIX m_View			 { XMMATRIX() };
	XMMATRIX m_Projection	 { XMMATRIX() };
	XMMATRIX m_ViewProjection{ XMMATRIX() };

	XMVECTOR m_Position		 { XMVECTOR() };
	XMVECTOR m_Target		 { XMVECTOR() };
	XMVECTOR m_Up			 { XMVECTOR() };

	XMMATRIX m_RotationX	 { XMMATRIX() };
	XMMATRIX m_RotationY	 { XMMATRIX() };
	XMMATRIX m_RotationMatrix{ XMMATRIX() };

	XMVECTOR m_Forward				{ XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f) };
	XMVECTOR m_Right				{ XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f) };
	XMVECTOR m_Upward				{ XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };

	XMVECTOR const m_DefaultPosition{ XMVectorSet(0.0f, 1.0f, -10.0f, 0.0f) };
	XMVECTOR const m_DefaultTarget	{ XMVectorSet(0.0f, 5.0f, 0.0f, 0.0f) };
	XMVECTOR const m_DefaultUp		{ XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };

	XMVECTOR const m_DefaultForward	{ XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f) };
	XMVECTOR const m_DefaultRight	{ XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f) };
	XMVECTOR const m_DefaultUpward	{ XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };

	float m_zNear{ 0.1f };
	float m_zFar{ 5'000'000.0f };

	float m_FoV{ XMConvertToRadians(45.0f) };

public:
	// For calling camera movement from keyboard inputs
	float MoveForwardBack{ 0.0f };
	float MoveRightLeft	 { 0.0f };
	float MoveUpDown	 { 0.0f };

	float m_Pitch{ 0.0f };
	float m_Yaw{ 0.0f };

	float GetCameraSpeed() const noexcept;
	void SetCameraSpeed(float NewSpeed) noexcept;

	void SetZNear(float NewZ) noexcept;
	void SetZFar(float NewZ) noexcept;
	inline float GetZNear() const noexcept;
	inline float GetZFar() const noexcept;

	// For GUI usage
	std::array<float, 3> m_CameraSlider;

	inline static float m_CameraSpeed{ 25.0f };

};
