#include "Window.hpp"
#include "../Utilities/Logger.hpp"
#include <dwmapi.h>

#pragma comment(lib, "dwmapi")

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

namespace { Window* window = 0; }
LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return window->WindowProc(hWnd, msg, wParam, lParam);
}

Window::Window(HINSTANCE hInstance) noexcept
{
    window = this;
    m_hInstance = hInstance;

    m_Display.Width = 1280.0f;
    m_Display.Height = 800.0f;
    m_Display.AspectRatio = m_Display.Width / m_Display.Height;

}

Window::~Window()
{
    Release();
}

bool Window::Initialize()
{
    if (bIsInitialized)
    {
        Logger::Log("Window is already initialized!", LogType::eWarning);
        return false;
    }

    WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.hInstance = m_hInstance;
    wcex.lpfnWndProc = MsgProc;
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpszClassName = m_WindowClass;
    wcex.hbrBackground = ::CreateSolidBrush(RGB(20, 20, 20));

    if (!::RegisterClassEx(&wcex))
    {
        ::MessageBoxA(nullptr, "Failed to register Window!", "Error", MB_OK);
        return false;
    }

    //test
    //constexpr auto borderless{ WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX };
    constexpr auto windowFlags{ WS_OVERLAPPED };
    m_WindowRect = { 0, 0, static_cast<LONG>(GetDisplay().Width), static_cast<LONG>(GetDisplay().Height)};
    ::AdjustWindowRect(&m_WindowRect, windowFlags, false);

    const int32_t width  = m_WindowRect.right  - m_WindowRect.left;
    const int32_t height = m_WindowRect.bottom - m_WindowRect.top;

    m_hWnd = ::CreateWindow(m_WindowClass, m_WindowName,
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          width, height,
                          0, 0,
                          m_hInstance, 0);

    if (!m_hWnd)
    {
        ::MessageBoxA(nullptr, "Failed to initialize HWND!", "Error", MB_OK);
        ::PostQuitMessage(0);
        return false;
    }

    const int32_t xPos = (::GetSystemMetrics(SM_CXSCREEN) - m_WindowRect.right) / 2;
    const int32_t yPos = (::GetSystemMetrics(SM_CYSCREEN) - m_WindowRect.bottom) / 2;
    ::SetWindowPos(GetHWND(), 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    const BOOL bDarkMode{ TRUE };
    ::DwmSetWindowAttribute(m_hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &bDarkMode, sizeof(bDarkMode));

    bIsInitialized = true;
    return true;
}

void Window::Show()
{
    if (!bIsInitialized)
        throw std::exception();

    //test
    //constexpr auto borderless{ WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX };
    //::SetWindowLongPtrW(m_hWnd, GWL_STYLE, static_cast<LONG>(borderless));

    //::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    ::ShowWindow(m_hWnd, SW_SHOW);
    ::SetForegroundWindow(m_hWnd);
    ::SetFocus(m_hWnd);
    ::UpdateWindow(m_hWnd);
}

void Window::ShowCursor() noexcept
{
    while (::ShowCursor(true) < 0)
        bCursorVisible = true;
}

void Window::HideCursor() noexcept
{
    while (::ShowCursor(false) >= 0)
        bCursorVisible = false;
}

void Window::Release()
{
    ::UnregisterClass(m_WindowClass, m_hInstance);
    //::DestroyWindow(m_hWnd);

    if (::window)
        window = nullptr;

    Logger::Log("Window released.");
}
