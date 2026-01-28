#include "ImGuiManager.h"
#include "../Graphics/SrvManager.h" 

void ImGuiManager::Initialize([[maybe_unused]]WinApp* winApp, [[maybe_unused]] DirectXCommon* dxCommon, [[maybe_unused]] SrvManager* srvManager) {
#ifdef USE_IMGUI



	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(winApp->GetHwnd());

	uint32_t index = srvManager_->Allocate();

	ImGui_ImplDX12_Init(
		dxCommon_->GetDevice(),
		DirectXCommon::kSwapChainBufferCount_,
		dxCommon_->GetRTVFormat(),
		srvManager_->descriptorHeap_.Get(), 
		srvManager_->GetCPUDescriptorHandle(index), 
		srvManager_->GetGPUDescriptorHandle(index)  
	);
#endif // USE_IMGUI
}

void ImGuiManager::Begin() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif // USE_IMGUI
}

void ImGuiManager::End() {
#ifdef USE_IMGUI
	ImGui::Render();
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif // USE_IMGUI
}

void ImGuiManager::Finalize() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // USE_IMGUI
}
