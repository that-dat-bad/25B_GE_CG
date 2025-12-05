#include "TDEngine.h" 
#include <Windows.h>

using namespace TDEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	///////////
	// 初期化
	///////////
	// エンジン初期化
	TDEngine::Initialize(L"testest");

	// シングルトン化されたインスタンスを取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	///////////
	// メインループ
	///////////
	while (true) {
		// エンジンの更新 (trueが返ってきたらループを抜ける)
		if (TDEngine::Update()) {
			break;
		}

		// ImGuiの受付開始
		imguiManager->Begin();

		//ここにゲームシーンの更新処理

		// ImGuiの受付終了
		imguiManager->End();

		// 描画開始
		dxCommon->PreDraw();

		//ここにゲームシーンの描画処理


		// ImGuiの描画
		imguiManager->Draw();

		// 描画終了
		dxCommon->PostDraw();
	}

	// エンジンの終了処理
	TDEngine::Finalize();

	return 0;
}