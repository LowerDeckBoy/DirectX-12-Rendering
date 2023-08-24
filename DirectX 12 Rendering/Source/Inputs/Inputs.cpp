#include "../Rendering/Camera.hpp"
#include "Inputs.hpp"
#include "../Core/Window.hpp"
#include "../Utilities/Utilities.hpp"

void Inputs::Initialize()
{
	ThrowIfFailed(DirectInput8Create(Window::GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&DxInput), NULL));
	ThrowIfFailed(DxInput->CreateDevice(GUID_SysKeyboard, &DxKeyboard, NULL));
	ThrowIfFailed(DxKeyboard->SetDataFormat(&c_dfDIKeyboard));
	ThrowIfFailed(DxKeyboard->SetCooperativeLevel(Window::GetHWND(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
	ThrowIfFailed(DxInput->CreateDevice(GUID_SysMouse, &DxMouse, NULL));
	ThrowIfFailed(DxMouse->SetDataFormat(&c_dfDIMouse));
	ThrowIfFailed(DxMouse->SetCooperativeLevel(Window::GetHWND(), DISCL_NONEXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND));
}

void Inputs::CameraInputs(Camera* pCamera, float DeltaTime)
{
	DIMOUSESTATE mouseState{};
	constexpr int keys{ 256 };
	std::array<BYTE, keys> keyboardState{};

	DxKeyboard->Acquire();
	DxMouse->Acquire();

	DxMouse->GetDeviceState(sizeof(mouseState), reinterpret_cast<LPVOID>(&mouseState));
	DxKeyboard->GetDeviceState(sizeof(keyboardState), reinterpret_cast<LPVOID>(&keyboardState));

	constexpr int state{ 0x80 };

	if (keyboardState.at(DIK_ESCAPE) & state)
	{
		// TODO: Add closing fullscreen state here later
		::PostMessage(Window::GetHWND(), WM_QUIT, 0, 0);
	}

	if (!mouseState.rgbButtons[1])
	{
		Window::ShowCursor();
		return;
	}

	Window::HideCursor();

	const float speed{ Camera::m_CameraSpeed * static_cast<float>(DeltaTime) };
	constexpr float intensity{ 0.001f };
	constexpr float upDownIntensity{ 0.75f };

	if ((mouseState.lX != DxLastMouseState.lX) || (mouseState.lY != DxLastMouseState.lY))
	{
		pCamera->m_Yaw += mouseState.lX * intensity;
		pCamera->m_Pitch += mouseState.lY * intensity;
		DxLastMouseState = mouseState;
	}
	if (keyboardState.at(DIK_W) & state)
	{
		pCamera->MoveForwardBack += speed;
	}
	if (keyboardState.at(DIK_S) & state)
	{
		pCamera->MoveForwardBack -= speed;
	}
	if (keyboardState.at(DIK_A) & state)
	{
		pCamera->MoveRightLeft -= speed;
	}
	if (keyboardState.at(DIK_D) & state)
	{
		pCamera->MoveRightLeft += speed;
	}
	if (keyboardState.at(DIK_Q) & state)
	{
		pCamera->MoveUpDown -= speed * upDownIntensity;
	}
	if (keyboardState.at(DIK_E) & state)
	{
		pCamera->MoveUpDown += speed * upDownIntensity;
	}
	if (keyboardState.at(DIK_R) & state)
	{
		pCamera->ResetCamera();
	}

}

void Inputs::Release() noexcept
{
	if (DxKeyboard)
	{
		DxKeyboard->Unacquire();
		DxKeyboard->Release();
		DxKeyboard = nullptr;
	}

	if (DxMouse)
	{
		DxMouse->Unacquire();
		DxMouse->Release();
		DxMouse = nullptr;
	}

	if (DxInput)
	{
		DxInput->Release();
		DxInput = nullptr;
	}

	DxLastMouseState = {};
}
