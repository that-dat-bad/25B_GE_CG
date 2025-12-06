#include"SceneManager.h"
#include"TDEngine.h"

#include <Windows.h>

using namespace TDEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	TDEngine::Initialize(L"2230_風船の灯火");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	SceneManager* sceneManager = nullptr;
	sceneManager = new SceneManager();
	sceneManager->Initialize();

	// メインループ
	while (true) {
		// エンジンの更新
		if (TDEngine::Update()) {
			break;
		}

		imguiManager->Begin();

		//ゲームシーンの更新処理
		sceneManager->Update();

		imguiManager->End();

		//描画前処理
		dxCommon->PreDraw();

		// 描画処理
		sceneManager->Draw();

		imguiManager->Draw();

		//描画後処理
		dxCommon->PostDraw();
	}

	TDEngine::Finalize();

	delete sceneManager;

	return 0;
}
