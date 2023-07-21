#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <windowsx.h>
#include <iostream>

// TODO: Get rid of statics 
// should work just fine without them
class Window
{
public:
	explicit Window(HINSTANCE hInstance) noexcept;
	Window(const Window&) = delete;
	virtual ~Window();

	bool Initialize();
	void Show();

	// For GUI usage
	static void ShowCursor() noexcept;
	static void HideCursor() noexcept;
	
	void Release();

	virtual LRESULT WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
	virtual void OnResize() = 0;

	inline static struct Display
	{
		float Width;
		float Height;
		float AspectRatio;
	} m_Display;

	[[nodiscard]]
	static HINSTANCE GetHInstance() noexcept { return m_hInstance;  }
	[[nodiscard]]
	static HWND GetHWND() noexcept { return m_hWnd; }
	[[nodiscard]]
	static inline Display GetDisplay() noexcept { return m_Display; }

	inline static bool bShouldResize{ false };

	inline static HWND m_hWnd{ nullptr };
private:
	inline static HINSTANCE m_hInstance{ nullptr };
	inline static RECT m_WindowRect{ };

	inline static LPCWSTR m_WindowName{ L"Main Window" };
	inline static LPCWSTR m_WindowClass{ L"Window" };

	inline static bool bCursorVisible{ true };

	inline static bool bIsInitialized{ false };

};
