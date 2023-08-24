#pragma once
#include "../Core/Window.hpp"
#include "../Utilities/Timer.hpp"
#include "../Rendering/Camera.hpp"
#include "../Core/Renderer.hpp"
#include "../Utilities/Logger.hpp"


class Engine : public Window
{
public:
	explicit Engine(HINSTANCE hInstance);
	~Engine();
	
	void Initialize();
	void Run();

private:
	LRESULT WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) final;
	void OnResize() final;

	void Release();

private:
	std::unique_ptr<Logger>   m_Logger;
	std::unique_ptr<Timer>	  m_Timer;	
	std::unique_ptr<Renderer> m_Renderer;
	std::unique_ptr<Camera>   m_Camera;

private:
	// Window states for window moving and resizing
	bool bAppPaused { false };
	bool bMinimized { false };
	bool bMaximized { false };
	bool bIsResizing{ false };

};

