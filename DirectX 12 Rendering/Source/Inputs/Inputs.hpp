#pragma once
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

class Camera;

class Inputs
{
public:
	static void Initialize();
	static void CameraInputs(Camera* pCamera, float DeltaTime);
	static void Release() noexcept;
	
private:
	inline static IDirectInputDevice8* DxKeyboard{};
	inline static IDirectInputDevice8* DxMouse{};
	inline static LPDIRECTINPUT8 DxInput{};
	inline static DIMOUSESTATE DxLastMouseState{};

};
