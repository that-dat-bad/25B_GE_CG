#include "TDEngine.h"
#include "SceneManager.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"

// Windowsアプリのエントリーポイント
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// 1. エンジンの初期化
	// ウィンドウタイトル、幅、高さを指定
	TDEngine::Initialize(L"TDEngine Ported Game", 1280, 720);

	// 2. シーンマネージャの生成と初期化
	SceneManager* sceneManager = new SceneManager();
	sceneManager->Initialize();

	// 3. メインループ
	while (TDEngine::Update()) {

		// --- 更新処理開始 ---

		// ImGui受付開始
		ImGuiManager::GetInstance()->Begin();

		// シーン更新
		sceneManager->Update();

		// --- 描画処理開始 ---

		// DirectX描画前処理 (画面クリア、バリア設定など)
		DirectXCommon::GetInstance()->PreDraw();

		// シーン描画 (コマンド積み込み)
		sceneManager->Draw();


		// ImGui終了処理
		ImGuiManager::GetInstance()->End();


		// ImGui描画 (コマンド積み込み)
		ImGuiManager::GetInstance()->Draw();

		// DirectX描画後処理 (コマンド実行、フリップ、待機)
		DirectXCommon::GetInstance()->PostDraw();
	}

	// 4. 終了処理
	delete sceneManager;
	TDEngine::Finalize();

	return 0;
}