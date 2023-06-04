#pragma once
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>
#include <array>
#include "../Core/Window.hpp"
#include "../Rendering/Camera.hpp"
#include "../Utils/Utilities.hpp"

//class Camera;

class Inputs
{
public:
	static void Initialize()
	{
		ThrowIfFailed(DirectInput8Create(Window::GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DxInput, NULL));
		ThrowIfFailed(DxInput->CreateDevice(GUID_SysKeyboard, &DxKeyboard, NULL));
		ThrowIfFailed(DxKeyboard->SetDataFormat(&c_dfDIKeyboard));
		ThrowIfFailed(DxKeyboard->SetCooperativeLevel(Window::GetHWND(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
		ThrowIfFailed(DxInput->CreateDevice(GUID_SysMouse, &DxMouse, NULL));
		ThrowIfFailed(DxMouse->SetDataFormat(&c_dfDIMouse));
		ThrowIfFailed(DxMouse->SetCooperativeLevel(Window::GetHWND(), DISCL_NONEXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND));
	}

	static void CameraInputs(Camera* pCamera, const float DeltaTime)
	{
		DIMOUSESTATE mouseState{};
		const int keys{ 256 };
		std::array<BYTE, keys> keyboardState{};

		DxKeyboard->Acquire();
		DxMouse->Acquire();

		DxMouse->GetDeviceState(sizeof(mouseState), (LPVOID)&mouseState);
		DxKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

		const int state{ 0x80 };

		if (keyboardState[DIK_ESCAPE] & state)
		{
			// Add closing fullscreen state here later
			::PostMessage(Window::GetHWND(), WM_DESTROY, 0, 0);
		}

		if (!mouseState.rgbButtons[1])
		{
			Window::ShowCursor();
			return;
		}

		Window::HideCursor();

		float speed{ Camera::m_CameraSpeed * static_cast<float>(DeltaTime) };
		const float intensity{ 0.001f };
		const float upDownIntensity{ 0.75f };

		if ((mouseState.lX != DxLastMouseState.lX) || (mouseState.lY != DxLastMouseState.lY))
		{
			pCamera->m_Yaw += mouseState.lX * intensity;
			pCamera->m_Pitch += mouseState.lY * intensity;
			DxLastMouseState = mouseState;
		}
		if (keyboardState[DIK_A] & state)
		{
			pCamera->MoveRightLeft -= speed;
		}
		if (keyboardState[DIK_D] & state)
		{
			pCamera->MoveRightLeft += speed;
		}
		if (keyboardState[DIK_W] & state)
		{
			pCamera->MoveForwardBack += speed;
		}
		if (keyboardState[DIK_S] & state)
		{
			pCamera->MoveForwardBack -= speed;
		}
		if (keyboardState[DIK_Q] & state)
		{
			pCamera->MoveUpDown -= speed * upDownIntensity;
		}
		if (keyboardState[DIK_E] & state)
		{
			pCamera->MoveUpDown += speed * upDownIntensity;
		}

	}

	static void Release()
	{
		if (DxKeyboard)
		{
			DxKeyboard->Unacquire();
			DxKeyboard = nullptr;
		}

		if (DxMouse)
		{
			DxMouse->Unacquire();
			DxMouse = nullptr;
		}

		if (DxInput)
		{
			DxInput->Release();
			DxInput = nullptr;
		}

		DxLastMouseState = {};
	}


private:
	inline static IDirectInputDevice8* DxKeyboard{};
	inline static IDirectInputDevice8* DxMouse{};
	inline static LPDIRECTINPUT8 DxInput{};
	inline static DIMOUSESTATE DxLastMouseState{};
};

