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
	void Initialize(WinApp* winApp, DirectXCommon* dxCommon, SrvManager* srvManager);

	void Begin();

	void End();

	void Finalize();

private:
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
};
