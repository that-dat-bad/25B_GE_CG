#pragma once
#ifdef USE_IMGUI
#include "../../../external/imgui/imgui.h"
#include "../../../external/imgui/imgui_impl_dx12.h"
#include "../../../external/imgui/imgui_impl_win32.h"
#endif // USE_IMGUI
#include "../Graphics/DirectXCommon.h"
#include "../base/WinApp.h"

class SrvManager; 

class ImGuiManager {
public:

	static ImGuiManager* GetInstance();
	~ImGuiManager() = default;
	// 初期化
	void Initialize(WinApp* winApp, DirectXCommon* dxCommon, SrvManager* srvManager);

	// 更新処理
	void Begin();

	// 描画処理
	void End();

	void Draw();

	// 終了処理
	void Finalize();

private:

	ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	const ImGuiManager& operator=(const ImGuiManager&) = delete;

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
};