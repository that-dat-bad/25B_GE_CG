#include "Game.h"
#include <cassert>

void Game::Initialize() {
	// --- 各種システムの初期化 ---
	winApp = new WinApp();
	winApp->Initialize();

	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	input = new Input();
	input->Initialize(GetModuleHandle(nullptr), winApp->GetHwnd());

	srvManager = new SrvManager();
	srvManager->Initialize(dxCommon);

	imguiManager = new ImGuiManager();
	imguiManager->Initialize(winApp, dxCommon, srvManager);

	audioManager = new AudioManager();
	audioManager->Initialize();

	sceneManager = new SceneManager();


	// マネージャ類の初期化
	TextureManager::GetInstance()->Initialize(dxCommon, srvManager);
	ModelManager::GetInstance()->Initialize(dxCommon);
	CameraManager::GetInstance()->Initialize();
	ParticleManager::GetInstance()->Initialize(dxCommon, srvManager);

	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

	object3dCommon = new Object3dCommon();
	object3dCommon->Initialize(dxCommon);

	// --- リソース読み込み ---
	TextureManager::GetInstance()->LoadTexture("assets/textures/uvchecker.png");
	ModelManager::GetInstance()->LoadModel("models/sphere.obj");
	alarmSound = audioManager->SoundLoadFile("assets/sounds/Alarm01.wav");

	// --- オブジェクト生成 ---


	// ライト設定 (b1, b2レジスタ用)
	directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	lightingSettingsResource = dxCommon->CreateBufferResource(sizeof(LightingSettings));
	lightingSettingsResource->Map(0, nullptr, reinterpret_cast<void**>(&lightingSettingsData));
	lightingSettingsData->lightingModel = 0; // Lambert
}

void Game::Update() {
	// システムの更新
	input->Update();
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
	if (input->triggerKey(DIK_ESCAPE)) {
		endRequest_ = true;
	}
}

void Game::Draw() {
	// 描画前処理
	dxCommon->PreDraw();
	srvManager->PreDraw();
	// 3Dオブジェクト描画
	object3dCommon->SetupCommonState();

	// 定数バッファをセット
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(4, lightingSettingsResource->GetGPUVirtualAddress());

	sceneManager->Draw();
	ParticleManager::GetInstance()->Draw();

	// ImGui描画
	imguiManager->End();

	// 描画後処理
	dxCommon->PostDraw();
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

	delete object3dCommon;
	delete spriteCommon;
	delete srvManager;
	delete input;
	delete dxCommon;
	delete winApp;
}