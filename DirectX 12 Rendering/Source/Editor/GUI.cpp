#include "GUI.hpp"
#include "../Utils/Utils.hpp"
#include "../Core/Window.hpp"
#include "../Core/Device.hpp"

//#include "../Utils/Timer.hpp"
#include "../Rendering/Camera.hpp"

GUI::~GUI()
{
	m_MainFont = nullptr;
	SafeRelease(m_Heap);
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void GUI::Initialize(Device* pDevice, Camera& refCamera)
{
	assert(m_Device = pDevice);
	//assert(m_Camera = refCamera);
	m_Camera = &refCamera;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& IO{ ImGui::GetIO() };
	//IO.Framerate
	//IO.ConfigFlags |= ImGuiConfigFlags_DockingEnabled;
	ImGuiStyle& Style{ ImGui::GetStyle() };
	Style.WindowRounding = 5.0f;
	Style.WindowBorderSize = 0.0f;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NumDescriptors = 1;
	ThrowIfFailed(pDevice->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_Heap.GetAddressOf())));

	ImGui_ImplWin32_Init(Window::GetHWND());
	ImGui_ImplDX12_Init(pDevice->GetDevice(),
						Device::FrameCount,
						pDevice->GetRTVFormat(),
						m_Heap.Get(),
						m_Heap.Get()->GetCPUDescriptorHandleForHeapStart(),
						m_Heap.Get()->GetGPUDescriptorHandleForHeapStart());

	const float fontSize = 16.0f;
	m_MainFont = IO.Fonts->AddFontFromFileTTF("Assets/Fonts/CascadiaCode-Bold.ttf", fontSize);
	//m_MainFont = IO.Fonts->AddFontFromFileTTF("Assets/Fonts/BohoSunshine2.ttf", fontSize);

}

void GUI::Begin()
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX12_NewFrame();
	ImGui::NewFrame();
	ImGui::PushFont(m_MainFont);
}

void GUI::End(ID3D12GraphicsCommandList* pCommandList)
{
	{
		ImGui::Begin("Performence");
		//ImGui::Text("FPS: %.2f", Timer::m_FrameCount)
		ImGui::Text("Resolution: %.0fx%.0f", Window::GetDisplay().Width, Window::GetDisplay().Height);
		ImGui::Text("Aspect Ratio: %.2f", Window::GetDisplay().AspectRatio);
		ImGui::End();
	}

	m_Camera->DrawGUI();
	//{
	//	ImGui::Begin("Camera");;
	//	if (ImGui::DragFloat3("Position", m_Camera->m_CameraSlider.data()))
	//	{
	//		m_Camera->SetPosition(m_Camera->m_CameraSlider);
	//		m_Camera->Update();
	//	}
	//	ImGui::End();
	//}

	ImGui::PopFont();
	ImGui::Render();

	ID3D12DescriptorHeap* ppHeaps[]{ m_Heap.Get() };
	pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList);
}
