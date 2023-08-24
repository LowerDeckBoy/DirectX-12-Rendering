#pragma once
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

class DeviceContext;
class Camera;
//class Timer;

class Editor
{
public:
	~Editor();

	void Initialize(DeviceContext* pDevice, Camera* pCamera);
	void Begin();
	// Goes before making Barrier Resource Transition for Present state
	void End(ID3D12GraphicsCommandList* pCommandList);

	void Logs();

	void Release();

private:
	DeviceContext* m_Device{ nullptr };
	Camera* m_Camera{ nullptr };
	
	ImFont* m_MainFont{ nullptr };

	bool bShowLogs{ false };

};
