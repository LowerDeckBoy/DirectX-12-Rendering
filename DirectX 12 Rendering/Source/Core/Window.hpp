#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif
#include <Windows.h>
#include <iostream>

static struct Display
{
	float Width{ 1280.0f };
	float Height{ 800.0f };
	float AspectRatio{ Width / Height };
} m_Display;


class Window
{
public:
	explicit Window(HINSTANCE hInstance);
	Window(const Window&) = delete;
	Window operator=(const Window&) = delete;
	~Window();

	bool Initialize();
	void Show();

	// void ShowCursor();
	// void HideCursor();
	
	//int Run();
	//void Destroy();

	static LRESULT WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


	[[nodiscard]]
	HINSTANCE GetHInstance() const { return m_hInstance;  }
	[[nodiscard]]
	HWND GetHWND() const { return m_hWnd; }
	[[nodiscard]]
	static inline Display GetDisplay() { return m_Display; }

private:
	HINSTANCE m_hInstance{ nullptr };
	HWND m_hWnd{ nullptr };
	RECT m_WindowRect{};

	LPCWSTR m_WindowName{ L"Main Window" };
	LPCWSTR m_WindowClass{ L"Window" };

	bool bIsInitialized{ false };


};

