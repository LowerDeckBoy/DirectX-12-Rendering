#include "GUI.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Core/Window.hpp"
#include "../Core/Renderer.hpp"
//#include "../Core/DeviceContext.hpp"

#include "../Utilities/Timer.hpp"
#include "../Rendering/Camera.hpp"
#include "../Utilities/MemoryUsage.hpp"
#include "../Utilities/Logger.hpp"


GUI::~GUI()
{
	Release();
}

void GUI::Initialize(DeviceContext* pDevice, Camera* pCamera)
{

	assert(m_Device = pDevice);
	m_Camera = pCamera;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& IO{ ImGui::GetIO() };
	ImGuiStyle& Style{ ImGui::GetStyle() };
	Style.WindowRounding = 5.0f;
	Style.WindowBorderSize = 0.0f;

	// Docking
	IO.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
	IO.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
	
	IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//IO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NumDescriptors = 1;
	ThrowIfFailed(pDevice->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_Heap.GetAddressOf())));
	m_Heap.Get()->SetName(L"ImGui Heap");

	//ImGui_ImplDX12_Init(pDevice->GetDevice(),
	//					DeviceContext::FrameCount,
	//					pDevice->GetRTVFormat(),
	//					m_Heap.Get(),
	//					m_Heap.Get()->GetCPUDescriptorHandleForHeapStart(),
	//					m_Heap.Get()->GetGPUDescriptorHandleForHeapStart());

	ImGui_ImplWin32_Init(Window::GetHWND());
	ImGui_ImplDX12_Init(pDevice->GetDevice(),
			DeviceContext::FRAME_COUNT,
			pDevice->GetRTVFormat(),
			pDevice->GetMainHeap()->GetHeap(),
			pDevice->GetMainHeap()->GetHeap()->GetCPUDescriptorHandleForHeapStart(),
			pDevice->GetMainHeap()->GetHeap()->GetGPUDescriptorHandleForHeapStart());

	constexpr float fontSize{ 16.0f };
	m_MainFont = IO.Fonts->AddFontFromFileTTF("Assets/Fonts/CascadiaCode-Bold.ttf", fontSize);

}

void GUI::Begin()
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX12_NewFrame();
	ImGui::NewFrame();
	ImGui::PushFont(m_MainFont);

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	
}

void GUI::End(ID3D12GraphicsCommandList* pCommandList)
{
	{
		ImGui::BeginMainMenuBar();
		// Calculate window sizes to get width for right size aligment
		// for closing button
		const auto region{ ImGui::GetContentRegionAvail() };
		const float rightSideAlignment { region.x - 20 };

		ImGui::MenuItem("File");
		ImGui::MenuItem("Window");
		ImGui::MenuItem("Log", nullptr, &bShowLogs);
		if (bShowLogs)
		{
			Logs();
		}

		ImGui::Text("| FPS: %.2d ms: %.2f", Timer::m_FPS, Timer::m_Miliseconds);
		ImGui::Text(" | ");
		MemoryUsage::ReadRAM();
		ImGui::Text("RAM: %.3f MB | ", MemoryUsage::MemoryInUse);

		ImGui::SetCursorPosX(rightSideAlignment);
		if (ImGui::Button(" X ", { 35, 0 }))
		{
			::PostQuitMessage(0);
		}

		ImGui::EndMainMenuBar();
	}

	// Main Window
	{

	}

	// Side panel
	{
		ImGui::Begin("UI");

		if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			m_Camera->DrawGUI();
		}
		ImGui::Checkbox("Draw Sky", &Renderer::bDrawSky);
		ImGui::Checkbox("Raster", &Renderer::bRaster);

		ImGui::End();
	}

	{

	}


	ImGui::PopFont();
	ImGui::Render();

	//ID3D12DescriptorHeap* ppHeaps[]{ m_Heap.Get() };
	//pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	ID3D12DescriptorHeap* ppHeaps[]{ m_Device->GetMainHeap()->GetHeap() };
	pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList);
}

void GUI::Logs()
{
	ImGui::Begin("Logs");

	ImGui::End();
}

void GUI::Release()
{
	m_MainFont = nullptr;

	SAFE_RELEASE(m_Heap);
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	Logger::Log("GUI released.");
}
