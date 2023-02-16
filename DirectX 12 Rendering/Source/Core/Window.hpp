#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif
#include <Windows.h>
#include <iostream>


class Window
{
public:
	Window(HINSTANCE hInstance);
	//explicit Window(HINSTANCE hInstance);
	Window(const Window&) = delete;
	//Window operator=(const Window&) = delete;
	virtual ~Window();

	bool Initialize();
	void Show();
	// For GUI usage
	//static void ShowCursor();
	//static void HideCursor();
	
	void Destroy();

	virtual LRESULT WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
	virtual void OnResize() = 0;

	inline static struct Display
	{
		float Width{ 1280.0f };
		float Height{ 800.0f };
		float AspectRatio{ Width / Height };
	} m_Display;

	[[nodiscard]]
	static HINSTANCE GetHInstance() { return m_hInstance;  }
	[[nodiscard]]
	static HWND GetHWND() { return m_hWnd; }
	[[nodiscard]]
	static inline Display GetDisplay() { return m_Display; }

	inline static bool bShouldResize{ false };

	inline static HWND m_hWnd{ nullptr };
private:
	inline static HINSTANCE m_hInstance{ nullptr };
	inline static RECT m_WindowRect{ };

	inline static LPCWSTR m_WindowName{ L"Main Window" };
	inline static LPCWSTR m_WindowClass{ L"Window" };

	inline static bool bIsInitialized{ false };

};

