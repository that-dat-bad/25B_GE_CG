#include "TDEngine.h" 
#include <Windows.h>
#include"D3DResourceLeakChecker.h"
using namespace TDEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	D3DResourceLeakChecker leakCheck;
	///////////
	// 初期化
	///////////
	// エンジン初期化
	TDEngine::Initialize(L"testest");

	Texture::LoadTexture("assets/textures/monsterBall.png");
	Sprite* sprite1 = Sprite::Create("assets/textures/monsterBall.png", { 300.0f, 300.0f });
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
		sprite1->Update();
		// ImGuiの受付終了
		imguiManager->End();

		// 描画開始
		dxCommon->PreDraw();

		//ここにゲームシーンの描画処理
		sprite1->Draw(dxCommon);
		// ImGuiの描画
		imguiManager->Draw();

		// 描画終了
		dxCommon->PostDraw();
	}
	delete sprite1;
	// エンジンの終了処理
	TDEngine::Finalize();

	return 0;
}