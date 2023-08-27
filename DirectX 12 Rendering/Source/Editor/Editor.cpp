#include "Editor.hpp"
#include "../Core/DeviceContext.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Core/Window.hpp"
#include "../Core/Renderer.hpp"

#include "../Utilities/Timer.hpp"
#include "../Utilities/MemoryUsage.hpp"
#include "../Utilities/Logger.hpp"
#include "../Rendering/Camera.hpp"


Editor::~Editor()
{
	Release();
}

void Editor::Initialize(DeviceContext* pDevice, Camera* pCamera)
{
	assert(m_Device = pDevice);
	assert(m_Camera = pCamera);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& IO{ ImGui::GetIO() };
	ImGuiStyle& Style{ ImGui::GetStyle() };
	Style.WindowRounding = 5.0f;
	Style.WindowBorderSize = 0.0f;

	// Docking
	IO.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
	IO.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
	IO.ConfigFlags  |= ImGuiConfigFlags_DockingEnable;
	//IO.ConfigFlags  |= ImGuiConfigFlags_ViewportsEnable;

	assert(ImGui_ImplWin32_Init(Window::GetHWND()));
	assert(ImGui_ImplDX12_Init(pDevice->GetDevice(),
			DeviceContext::FRAME_COUNT,
			pDevice->GetRTVFormat(),
			pDevice->GetMainHeap()->GetHeap(),
			pDevice->GetMainHeap()->GetHeap()->GetCPUDescriptorHandleForHeapStart(),
			pDevice->GetMainHeap()->GetHeap()->GetGPUDescriptorHandleForHeapStart()));

	constexpr float fontSize{ 16.0f };
	m_MainFont = IO.Fonts->AddFontFromFileTTF("Assets/Fonts/CascadiaCode-Bold.ttf", fontSize);

}

void Editor::Begin()
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX12_NewFrame();
	ImGui::NewFrame();
	ImGui::PushFont(m_MainFont);
}

void Editor::End(ID3D12GraphicsCommandList* pCommandList)
{
	{
		ImGui::BeginMainMenuBar();

		// Calculate window sizes to get width for right size aligment
		const auto region{ ImGui::GetContentRegionAvail() };

		ImGui::MenuItem("File");
		ImGui::MenuItem("Window");

		const float rightSideAlignment { region.x - 430.f };
		ImGui::SetCursorPosX(rightSideAlignment);
		MemoryUsage::ReadRAM();
		const uint32_t VRAM{ DeviceContext::QueryAdapterMemory() };
		ImGui::Text("FPS: %d ms: %.2f | Memory: %.3f MB | VRAM: %u MB", Timer::m_FPS, Timer::m_Miliseconds, MemoryUsage::MemoryInUse, VRAM);

		ImGui::EndMainMenuBar();
	}

	// Side panel
	{
		ImGui::Begin("UI");

		if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			m_Camera->DrawGUI();
		}
		ImGui::Checkbox("Draw Sky", &Renderer::bDrawSky);
		ImGui::SameLine();
		ImGui::Checkbox("Deferred", &Renderer::bDeferred);
		ImGui::SameLine();
		ImGui::Checkbox("V-Sync", &Renderer::bVsync);

		ImGui::End();
	}

	ImGui::PopFont();
	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList);
}

void Editor::Release()
{
	m_MainFont = nullptr;

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	Logger::Log("GUI released.");
}
