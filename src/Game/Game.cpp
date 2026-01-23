#include "Game.h"
#include <cassert>

void Game::Initialize() {
	// 1. 基盤システムの初期化
	winApp = std::make_unique<WinApp>();
	winApp->Initialize();
	DirectXCommon::GetInstance()->Initialize(winApp.get());
	Input::GetInstance()->Initialize(GetModuleHandle(nullptr), winApp->GetHwnd());

	srvManager = std::make_unique<SrvManager>();
	srvManager->Initialize(DirectXCommon::GetInstance());

	// 2. マネージャ類の初期化 (SceneManager より先に！ )
	SpriteCommon::GetInstance()->Initialize(DirectXCommon::GetInstance());
	TextureManager::GetInstance()->Initialize(DirectXCommon::GetInstance(), srvManager.get());
	ModelManager::GetInstance()->Initialize(DirectXCommon::GetInstance());
	CameraManager::GetInstance()->Initialize();
	ParticleManager::GetInstance()->Initialize(DirectXCommon::GetInstance(), srvManager.get());

	Object3dCommon::GetInstance()->Initialize(DirectXCommon::GetInstance());

	imguiManager = std::make_unique<ImGuiManager>();
	imguiManager->Initialize(winApp.get(), DirectXCommon::GetInstance(), srvManager.get());

	// 3. シーンマネージャの生成 (全ての準備が整ってから)
	sceneManager = std::make_unique<SceneManager>(); // 内部でシーンの Initialize が走る

}

void Game::Update() {
	// システムの更新
	Input::GetInstance()->Update();
	imguiManager->Begin();

	sceneManager->Update();
	// カメラ更新
	CameraManager::GetInstance()->Update();



	ParticleManager::GetInstance()->Update();


#ifdef USE_IMGUI

	ImGui::Begin("Camera Controls");

	// アクティブなカメラを取得
	Camera* camera = CameraManager::GetInstance()->GetActiveCamera();
	if (camera) {
		// 現在の値を取得
		Vector3 rotate = camera->GetRotate();
		Vector3 translate = camera->GetTranslate();

		// ImGuiで値を操作 (DragFloat3)
		// 第3引数は感度（ドラッグ時の変化量）です
		ImGui::DragFloat3("Rotate", &rotate.x, 0.01f);
		ImGui::DragFloat3("Translate", &translate.x, 0.1f);

		// 変更した値をカメラにセットし直す
		camera->SetRotate(rotate);
		camera->SetTranslate(translate);
	}
	ImGui::End();
#endif // USE_IMGUI

	// 終了リクエストの例 (Escキーで終了など)
	if (Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
		endRequest_ = true;
	}
}

void Game::Draw() {
	// 描画前処理
	DirectXCommon::GetInstance()->PreDraw();
	srvManager->PreDraw();
	// 3Dオブジェクト描画
	Object3dCommon::GetInstance()->SetupCommonState();


	sceneManager->Draw();
	ParticleManager::GetInstance()->Draw();

	// ImGui描画
	imguiManager->End();

	// 描画後処理
	DirectXCommon::GetInstance()->PostDraw();
}

void Game::Run() {
	// メインループ
	while (true) {
		// Windows メッセージの処理
		if (winApp->ProcessMessage()) {
			break;
		}

		// ゲーム更新
		Update();

		// 終了リクエストが来たら抜ける
		if (IsEndRequest()) {
			break;
		}

		// 描画
		Draw();
	}
}

void Game::Finalize() {
	// クリーンアップ処理
	if (imguiManager) {
		imguiManager->Finalize();
	}

	if (audioManager) {
		audioManager->SoundUnload(&alarmSound);
		audioManager->Finalize();
	}

	// シングルトン類
	ModelManager::GetInstance()->Finalize();
	TextureManager::GetInstance()->Finalize();
	CameraManager::GetInstance()->Finalize();
	ParticleManager::GetInstance()->Finalize();

}