#pragma once
#include "../Core/Window.hpp"
#include "../Utils/Timer.hpp"
#include "../Rendering/Camera.hpp"
#include "../Core/Renderer.hpp"


class Engine : public Window
{
public:
	//Engine() {} 
	explicit Engine(HINSTANCE hInstance);
	~Engine();
	
	void Initialize();
	void Run();

private:
	LRESULT WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
	void OnResize() override;

	void OnDestroy();

private:
	//std::unique_ptr<Timer> m_Timer;
	std::unique_ptr<Renderer> m_Renderer;
	std::unique_ptr<Camera> m_Camera;

private:
	// Window states for window moving and resizing
	bool bAppPaused{ false };
	bool bMinimized{ false };
	bool bMaximized{ false };
	bool bIsResizing{ false };

};

