#include "App.hpp"

App::App(HINSTANCE hInstance)
{
	m_Renderer = std::make_unique<Renderer>(hInstance);
}

App::~App()
{
	Release();
}

void App::Initialize()
{
	m_Renderer->Initialize();
}

int App::Run()
{
	MSG msg{};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_Renderer->Update();
			m_Renderer->Draw();
		}
	}
	
	return static_cast<int>(msg.wParam);
}


void App::Release()
{
	
}
