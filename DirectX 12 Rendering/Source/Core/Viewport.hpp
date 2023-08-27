#pragma once
#include <d3d12.h>

class DeviceContext;
class Window;

// TODO:
class Viewport
{
public:
	Viewport();
	//Viewport(uint32_t Width, uint32_t Height);
	~Viewport();

	void Initialize();
	void Set();
	void SetViewport();
	void SetScissor();
	
	void OnResize();

	void Release();

	D3D12_VIEWPORT	GetViewport() const noexcept;
	D3D12_RECT		GetScrissor() const noexcept;

private:

	D3D12_VIEWPORT m_Viewport{};
	D3D12_RECT m_Scrissor{};



};

