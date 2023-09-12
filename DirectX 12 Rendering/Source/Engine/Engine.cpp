#include "Engine.hpp"
#include "../Inputs/Inputs.hpp"
#include <imgui_impl_win32.h>


Engine::Engine(HINSTANCE hInstance) : Window(hInstance)
{
	m_Timer		= std::make_unique<Timer>();
	m_Renderer	= std::make_unique<Renderer>();
	m_Camera	= std::make_unique<Camera>();
}

Engine::~Engine()
{
	Release();
}

void Engine::Initialize()
{
	m_Logger = std::make_unique<Logger>();
	m_Logger->Init();

	Window::Initialize();
	m_Camera->Initialize(Window::GetDisplay().AspectRatio);

	m_Renderer->Initialize(m_Camera.get());
	m_Timer->Initialize();

}

void Engine::Run()
{
	// If app is ready to render show actual window
	// otherwise window would be blank until 
	// resources are ready to render
	Window::Show();

	// Stop timer on Resize event 
	// so Backbuffer resizing can't be done
	// without ongoing frame rendering
	//m_Timer->Start();

	Inputs::Initialize();
	// Ensure Timer clean start
	m_Timer->Reset();
	m_Camera->ResetCamera();

	MSG msg{};

	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			m_Timer->Tick();
			m_Timer->GetFrameStats();

			if (!bAppPaused)
			{
				Inputs::CameraInputs(m_Camera.get(), m_Timer->DeltaTime());
				m_Renderer->Update();
				m_Renderer->Render();
				m_Camera->Update();
			}
			else
				::Sleep(100);
		}
	}
}

void Engine::OnResize()
{
	m_Renderer->OnResize();
	m_Camera->OnAspectRatioChange(Window::GetDisplay().AspectRatio);
}

void Engine::Release()
{
	m_Timer->Stop();
	m_Renderer->GetDeviceContext()->WaitForGPU();
	Inputs::Release();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT Engine::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
		return true; 

	switch (Msg)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			bAppPaused = true;
			m_Timer->Stop();
		}
		else
		{
			bAppPaused = false;
			m_Timer->Start();
		}
		return 0;
	}
	case WM_SIZE:
	{
		m_Display.Width = static_cast<float>(LOWORD(lParam));
		m_Display.Height = static_cast<float>(HIWORD(lParam));
		m_Display.AspectRatio = (m_Display.Width / m_Display.Height);

		if (!m_Renderer)
		{
			return 0;
		}
		if (wParam == SIZE_MINIMIZED)
		{
			bAppPaused = true;
			bMinimized = true;
			bMaximized = false;

		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			bAppPaused = false;
			bMinimized = false;
			bMaximized = true;
			OnResize();
		}
		else if (wParam == SIZE_RESTORED)
		{
			if (bMinimized)
			{
				bAppPaused = false;
				bMinimized = false;
			}
			else if (bMaximized)
			{
				bAppPaused = false;
				bMaximized = false;
			}
			OnResize();
		}

		return 0;
	}
	case WM_ENTERSIZEMOVE:
	{
		bAppPaused = true;
		bIsResizing = true;
		m_Timer->Stop();

		return 0;
	}
	case WM_EXITSIZEMOVE:
	{
		bAppPaused = false;
		bIsResizing = false;

		//OnResize();
		m_Timer->Start();

		return 0;
	}
	case WM_CLOSE:
	case WM_DESTROY:
	{
		::PostQuitMessage(0);
		return 0;
	}
	}
	
	return ::DefWindowProcW(hWnd, Msg, wParam, lParam);
}
