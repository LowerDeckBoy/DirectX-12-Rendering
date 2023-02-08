#pragma once
#include "Device.hpp"
#include "Window.hpp"
#include <memory>

class Renderer : public Device
{
public:
	explicit Renderer(HINSTANCE hInstance);
	~Renderer();

	void Initialize();

	void Update();
	void Draw();

	void RecordCommandLists();
	void WaitForPreviousFrame();

private:
	std::unique_ptr<Window> m_Window;

};

