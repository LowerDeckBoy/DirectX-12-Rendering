#include "Engine.hpp"
#include <imgui.h>
//#include <imgui_impl_win32.h>
//#include <imgui_impl_dx12.h>
#include "../Inputs/Inputs.hpp"

Engine::Engine(HINSTANCE hInstance) : Window(hInstance)
{
	//m_Timer = std::make_unique<Timer>();
	m_Renderer = std::make_unique<Renderer>();
	m_Camera = std::make_unique<Camera>();

}

Engine::~Engine()
{
	OnDestroy();
}

void Engine::Initialize()
{
	Window::Initialize();
	m_Camera->Initialize(Window::GetDisplay().AspectRatio);
	m_Camera->ResetCamera();
	m_Renderer->Initialize(*m_Camera);
	Timer::Initialize();
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

	MSG msg{};

	// Ensure Timer clean start
	Inputs::Initialize();
	Timer::Reset();
	//m_Timer->Reset();

	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		Timer::Tick();
		Timer::GetFrameStats();

		if (!bAppPaused)
		{
			Inputs::CameraInputs(m_Camera.get(), Timer::DeltaTime());
			m_Renderer->Update(m_Camera->GetViewProjection());
			m_Renderer->Draw(m_Camera.get());
			m_Camera->Update();
		}
		else
			::Sleep(100);
	}
}

void Engine::OnResize()
{
	//OutputDebugStringA("Resize event occured.\n");
	m_Renderer->OnResize();
	m_Camera->OnAspectRatioChange(Window::GetDisplay().AspectRatio);
}

void Engine::OnDestroy()
{
	Timer::Stop();
	m_Renderer->WaitForGPU();
	Inputs::Release();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT Engine::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	// For debug purposes uncomment OutputDebugStringA
	switch (msg)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			//OutputDebugStringA("WA_INACTIVE\n");
			bAppPaused = true;
			Timer::Stop();
			//OutputDebugStringA("Timer stopped\n");
		}
		else
		{
			bAppPaused = false;
			Timer::Start();
			//OutputDebugStringA("Timer started\n");
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
			//OutputDebugStringA("Failed to get Renderer on resize event!\n");
			return 0;
		}
		if (wParam == SIZE_MINIMIZED)
		{
			//OutputDebugStringA("SIZE_MINIMIZED\n");
			bAppPaused = true;
			bMinimized = true;
			bMaximized = false;

		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			//OutputDebugStringA("SIZE_MAXIMIZED\n");
			bAppPaused = false;
			bMinimized = false;
			bMaximized = true;
			OnResize();
		}
		else if (wParam == SIZE_RESTORED)
		{
			//OutputDebugStringA("SIZE_RESTORED\n");
			if (bMinimized)
			{
				bAppPaused = false;
				bMinimized = false;
				//OnResize();
			}
			else if (bMaximized)
			{
				bAppPaused = false;
				bMaximized = false;
				//OnResize();
			}
			else
			{
				//OnResize();
			}
			//else if (bIsResizing)
			//{
			//	//OnResize();
			//}
			OnResize();
		}
	}
	return 0;

	case WM_ENTERSIZEMOVE:
	{
		//OutputDebugStringA("WM_ENTERSIZEMOVE\n");
		bAppPaused = true;
		bIsResizing = true;
		Timer::Stop();
		//OutputDebugStringA("Timer stopped\n");

		return 0;
	}

	case WM_EXITSIZEMOVE:
	{
		//OutputDebugStringA("WM_EXITSIZEMOVE\n");
		bAppPaused = false;
		bIsResizing = false;
		Timer::Start();
		//OutputDebugStringA("Timer started\n");

		OnResize();

		return 0;
	}
	
	case WM_CLOSE:
	case WM_DESTROY:
	{
		::PostQuitMessage(0);
		return 0;
	}
	//default:
	//	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
	}
	
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
