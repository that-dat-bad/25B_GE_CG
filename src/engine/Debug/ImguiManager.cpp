#include "ImGuiManager.h"
#include "../Graphics/SrvManager.h" 
using namespace TDEngine;

void ImGuiManager::Initialize([[maybe_unused]]WinApp* winApp, [[maybe_unused]] DirectXCommon* dxCommon, [[maybe_unused]] SrvManager* srvManager) {
#ifdef USE_IMGUI



	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	// ImGuiのコンテキスト生成
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// Win32初期化
	ImGui_ImplWin32_Init(winApp->GetHwnd());

	// --- SRVの確保 ---
	// ImGui用のSRVインデックスを1つ確保する
	uint32_t index = srvManager_->Allocate();

	// DX12初期化
	ImGui_ImplDX12_Init(
		dxCommon_->GetDevice(),
		DirectXCommon::kSwapChainBufferCount_,
		dxCommon_->GetRTVFormat(),
		srvManager_->descriptorHeap_.Get(), // SRVヒープ
		srvManager_->GetCPUDescriptorHandle(index), // CPUハンドル
		srvManager_->GetGPUDescriptorHandle(index)  // GPUハンドル
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

void ImGuiManager::Draw() {
#ifdef USE_IMGUI
	// 実際の描画コマンド発行
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif
}

void ImGuiManager::End() {
#ifdef USE_IMGUI
	// 内部の処理を確定
	ImGui::Render();
#endif
}

void ImGuiManager::Finalize() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // USE_IMGUI
}

ImGuiManager* ImGuiManager::GetInstance() {
	static ImGuiManager* instance = nullptr;
	if (instance == nullptr) {
		instance = new ImGuiManager();
	}
	return instance;
}