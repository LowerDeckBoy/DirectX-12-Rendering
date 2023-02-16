#pragma once
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <wrl.h>

class Device;

class GUI
{
public:
	~GUI();

	void Initialize(Device* pDevice);
	void Begin();
	// Goes before making Barrier Resource Transition
	void End(ID3D12GraphicsCommandList* pCommandList);

private:
	Device* m_Device{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
	// Can't be unique_ptr
	ImFont* m_MainFont;

};

