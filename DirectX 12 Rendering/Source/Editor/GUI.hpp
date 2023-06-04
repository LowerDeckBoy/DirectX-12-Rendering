#pragma once
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <wrl.h>

class Device;
class Camera;
//class Timer;

class GUI
{
public:
	~GUI();

	void Initialize(Device* pDevice, Camera& refCamera);
	void Begin();
	// Goes before making Barrier Resource Transition
	void End(ID3D12GraphicsCommandList* pCommandList);

private:
	Device* m_Device{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
	
	ImFont* m_MainFont;

	Camera* m_Camera;
};

