#include "Engine.hpp"
#include <imgui.h>
#include "../Inputs/Inputs.hpp"


Engine::Engine(HINSTANCE hInstance) : Window(hInstance)
{
	//m_Timer = std::make_unique<Timer>();
	m_Renderer = std::make_unique<Renderer>();
	m_Camera = std::make_unique<Camera>();

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
	m_Camera->ResetCamera();
	m_Renderer->Initialize(m_Camera.get());
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

	// Ensure Timer clean start
	Inputs::Initialize();
	Timer::Reset();

	MSG msg{};

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
			m_Renderer->Update();
			m_Renderer->Draw();
			m_Camera->Update();
		}
		else
			::Sleep(100);
	}

}

void Engine::OnResize()
{
	m_Renderer->OnResize();
	m_Camera->OnAspectRatioChange(Window::GetDisplay().AspectRatio);
}

void Engine::Release()
{
	Timer::Stop();
	m_Renderer->GetDeviceContext()->WaitForGPU();
	Inputs::Release();
}

//test
/*
void AdjustRect(RECT& Rect)
{
	auto monitor = ::MonitorFromWindow(Window::m_hWnd, MONITOR_DEFAULTTONULL);
	if (!monitor) {
		return;
	}

	MONITORINFO monitor_info{};
	monitor_info.cbSize = sizeof(monitor_info);
	if (!::GetMonitorInfoW(monitor, &monitor_info)) {
		return;
	}

	// when maximized, make the client area fill just the monitor (without task bar) rect,
	// not the whole window rect which extends beyond the monitor.
	Rect = monitor_info.rcWork;
}

LRESULT HitTest(POINT Cursor)
{
	const POINT border{
		::GetSystemMetrics(SM_CXFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER),
		::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER)
	};
	RECT window;
	if (!::GetWindowRect(Window::m_hWnd, &window)) {
		return HTNOWHERE;
	}

	auto borderless_drag = true;
	const auto drag = borderless_drag ? HTCAPTION : HTCLIENT;

	enum region_mask {
		client = 0b0000,
		left = 0b0001,
		right = 0b0010,
		top = 0b0100,
		bottom = 0b1000,
	};

	const auto result =
		left * (Cursor.x < (window.left + border.x)) |
		right * (Cursor.x >= (window.right - border.x)) |
		top * (Cursor.y < (window.top + border.y)) |
		bottom * (Cursor.y >= (window.bottom - border.y));

	auto borderless_resize = true;
	switch (result) {
	case left: return borderless_resize ? HTLEFT : drag;
	case right: return borderless_resize ? HTRIGHT : drag;
	case top: return borderless_resize ? HTTOP : drag;
	case bottom: return borderless_resize ? HTBOTTOM : drag;
	case top | left: return borderless_resize ? HTTOPLEFT : drag;
	case top | right: return borderless_resize ? HTTOPRIGHT : drag;
	case bottom | left: return borderless_resize ? HTBOTTOMLEFT : drag;
	case bottom | right: return borderless_resize ? HTBOTTOMRIGHT : drag;
	case client: return drag;
	default: return HTNOWHERE;
	}
}
*/
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT Engine::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
		return true;

	switch (Msg)
	{
		/*
	case WM_NCCREATE:
	{
		auto userdata = reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams;
		// store window instance pointer in window user data
		::SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(userdata));
	}
	case WM_NCCALCSIZE: {
		if (wParam == TRUE) {
			auto& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
			AdjustRect(params.rgrc[0]);
			return 0;
		}
		break;
	}
	case WM_NCHITTEST: {
		// When we have no border or title bar, we need to perform our
		// own hit testing to allow resizing and moving.
		//return HitTest(POINT{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });

		break;
	}
	*/
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			bAppPaused = true;
			Timer::Stop();
		}
		else
		{
			bAppPaused = false;
			Timer::Start();
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

		return 0;
	}
	case WM_ENTERSIZEMOVE:
	{
		bAppPaused = true;
		bIsResizing = true;
		Timer::Stop();

		return 0;
	}

	case WM_EXITSIZEMOVE:
	{
		bAppPaused = false;
		bIsResizing = false;

		//OnResize();
		Timer::Start();

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
