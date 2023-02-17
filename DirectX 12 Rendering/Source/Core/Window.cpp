#include "Window.hpp"
#include <dwmapi.h>
//#include <Uxtheme.h>
//#pragma comment(lib, "uxtheme")
#include <imgui_impl_win32.h>

#pragma comment(lib, "dwmapi")

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

namespace { Window* window = 0; }
LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return window->WindowProc(hWnd, msg, wParam, lParam);
}

Window::Window(HINSTANCE hInstance)
{
    window = this;
    m_hInstance = hInstance;
}

Window::~Window()
{
    Destroy();
}

bool Window::Initialize()
{
    if (bIsInitialized)
        return false;

    WNDCLASSEX wcex{};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.hInstance = m_hInstance;
    wcex.lpfnWndProc = MsgProc;
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpszClassName = m_WindowClass;
    wcex.hbrBackground = static_cast<HBRUSH>(::CreateSolidBrush(RGB(20, 20, 20)));

    if (!::RegisterClassEx(&wcex))
    {
        ::MessageBoxA(nullptr, "Failed to register Window!", "Error", MB_OK);
        return false;
    }
    
    m_WindowRect = { 0, 0, static_cast<LONG>(GetDisplay().Width), static_cast<LONG>(GetDisplay().Height)};
    ::AdjustWindowRect(&m_WindowRect, WS_OVERLAPPEDWINDOW, false);
    int width = m_WindowRect.right - m_WindowRect.left;
    int height = m_WindowRect.bottom - m_WindowRect.top;

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

    int xPos = (::GetSystemMetrics(SM_CXSCREEN) - m_WindowRect.right) / 2;
    int yPos = (::GetSystemMetrics(SM_CYSCREEN) - m_WindowRect.bottom) / 2;
    SetWindowPos(GetHWND(), 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    BOOL bDarkMode = TRUE;
    ::DwmSetWindowAttribute(m_hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &bDarkMode, sizeof(bDarkMode));

    bIsInitialized = true;
    return true;
}

void Window::Show()
{
    if (!bIsInitialized)
        throw std::exception();

    //::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    ::ShowWindow(m_hWnd, SW_SHOW);
    ::SetForegroundWindow(m_hWnd);
    //::SetFocus(m_hWnd);
    ::UpdateWindow(m_hWnd);
}

void Window::Destroy()
{
    ::DestroyWindow(m_hWnd);
    ::UnregisterClass(m_WindowClass, m_hInstance);
}
