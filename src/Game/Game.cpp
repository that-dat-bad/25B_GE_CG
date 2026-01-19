#include "Game.h"
#include <cassert>

void Game::Initialize() {
	// --- 各種システムの初期化 ---
	winApp = new WinApp();
	winApp->Initialize();

	//DirectXCommon::GetInstance() = new DirectXCommon();
	//DirectXCommon::GetInstance()->Initialize(winApp);
	DirectXCommon::GetInstance()->Initialize(winApp);

	Input::GetInstance()->Initialize(GetModuleHandle(nullptr), winApp->GetHwnd());

	srvManager = new SrvManager();
	srvManager->Initialize(DirectXCommon::GetInstance());

	imguiManager = new ImGuiManager();
	imguiManager->Initialize(winApp, DirectXCommon::GetInstance(), srvManager);

	audioManager = new AudioManager();
	audioManager->Initialize();

	sceneManager = new SceneManager();


	// マネージャ類の初期化
	TextureManager::GetInstance()->Initialize(DirectXCommon::GetInstance(), srvManager);
	ModelManager::GetInstance()->Initialize(DirectXCommon::GetInstance());
	CameraManager::GetInstance()->Initialize();
	ParticleManager::GetInstance()->Initialize(DirectXCommon::GetInstance(), srvManager);
	Object3dCommon::GetInstance()->Initialize(DirectXCommon::GetInstance());

	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(DirectXCommon::GetInstance());


	// --- リソース読み込み ---
	TextureManager::GetInstance()->LoadTexture("assets/textures/uvchecker.png");
	ModelManager::GetInstance()->LoadModel("models/sphere.obj");
	alarmSound = audioManager->SoundLoadFile("assets/sounds/Alarm01.wav");

	// --- オブジェクト生成 ---


	// ライト設定 (b1, b2レジスタ用)
	directionalLightResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(DirectionalLight));
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	lightingSettingsResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(LightingSettings));
	lightingSettingsResource->Map(0, nullptr, reinterpret_cast<void**>(&lightingSettingsData));
	lightingSettingsData->lightingModel = 0; // Lambert
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

	// --- ImGui によるライト調整---
	ImGui::Begin("Lighting Settings");
	ImGui::ColorEdit4("Light Color", &directionalLightData->color.x);
	ImGui::DragFloat3("Light Direction", &directionalLightData->direction.x, 0.01f, -1.0f, 1.0f);
	directionalLightData->direction = MyMath::Normalize(directionalLightData->direction);
	ImGui::DragFloat("Intensity", &directionalLightData->intensity, 0.01f, 0.0f, 10.0f);
	const char* models[] = { "Lambert", "Half-Lambert" };
	ImGui::Combo("Model Select", &lightingSettingsData->lightingModel, models, 2);
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
	imguiManager->Finalize();
	delete imguiManager;

	audioManager->SoundUnload(&alarmSound);
	audioManager->Finalize();
	delete audioManager;


	// シングルトン類
	ModelManager::GetInstance()->Finalize();
	TextureManager::GetInstance()->Finalize();
	CameraManager::GetInstance()->Finalize();
	ParticleManager::GetInstance()->Finalize();

	delete spriteCommon;
	delete srvManager;
	delete winApp;
}