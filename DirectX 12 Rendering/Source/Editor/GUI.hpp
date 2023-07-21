#pragma once
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <wrl.h>

#include <string>

class DeviceContext;
class Camera;
//class Timer;

class GUI
{
public:
	~GUI();

	void Initialize(DeviceContext* pDevice, Camera* pCamera);
	void Begin();
	// Goes before making Barrier Resource Transition for Present state
	void End(ID3D12GraphicsCommandList* pCommandList);

	void Logs();

	void Release();

private:
	DeviceContext* m_Device{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
	
	ImFont* m_MainFont{ nullptr };

	Camera* m_Camera{ nullptr };

	bool bShowLogs{ false };
};

// TEST
class GUIObject
{
public:
	void Draw()
	{
		if (ImGui::CollapsingHeader(m_Name.c_str()))
		{

		}
	}

	void SetName(const std::string_view& Name)
	{
		m_Name = Name.data();
	}

private:
	std::string m_Name;



};
