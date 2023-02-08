#pragma once
#include "Renderer.hpp"
//#include <memory>

class App
{
public:
	explicit App(HINSTANCE hInstance);
	App(const App&) = delete;
	App operator=(const App&) = delete;
	~App();

	void Initialize();
	int Run();
	void Release();

private:

	std::unique_ptr<Renderer> m_Renderer;


};

