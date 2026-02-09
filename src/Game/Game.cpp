#include "Game.h"
#include <cassert>

void Game::Initialize() {
	winApp = std::make_unique<WinApp>();
	winApp->Initialize();
	DirectXCommon::GetInstance()->Initialize(winApp.get());
	Input::GetInstance()->Initialize(GetModuleHandle(nullptr), winApp->GetHwnd());

	srvManager = std::make_unique<SrvManager>();
	srvManager->Initialize(DirectXCommon::GetInstance());

	SpriteCommon::GetInstance()->Initialize(DirectXCommon::GetInstance());
	TextureManager::GetInstance()->Initialize(DirectXCommon::GetInstance(), srvManager.get());
	ModelManager::GetInstance()->Initialize(DirectXCommon::GetInstance());
	CameraManager::GetInstance()->Initialize();
	ParticleManager::GetInstance()->Initialize(DirectXCommon::GetInstance(), srvManager.get());

	Object3dCommon::GetInstance()->Initialize(DirectXCommon::GetInstance());

	imguiManager = std::make_unique<ImGuiManager>();
	imguiManager->Initialize(winApp.get(), DirectXCommon::GetInstance(), srvManager.get());

	sceneManager = std::make_unique<SceneManager>(); 

	AudioManager::GetInstance()->Initialize();
}

void Game::Update() {
	Input::GetInstance()->Update();
	imguiManager->Begin();

	sceneManager->Update();
	CameraManager::GetInstance()->Update();



	ParticleManager::GetInstance()->Update();


#ifdef USE_IMGUI

	ImGui::Begin("Camera Controls");

	Camera* camera = CameraManager::GetInstance()->GetActiveCamera();
	if (camera) {
		Vector3 rotate = camera->GetRotate();
		Vector3 translate = camera->GetTranslate();

		ImGui::DragFloat3("Rotate", &rotate.x, 0.01f);
		ImGui::DragFloat3("Translate", &translate.x, 0.1f);

		camera->SetRotate(rotate);
		camera->SetTranslate(translate);
	}
	ImGui::End();
#endif // USE_IMGUI

	if (Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
		endRequest_ = true;
	}
}

void Game::Draw() {
	DirectXCommon::GetInstance()->PreDraw();
	srvManager->PreDraw();
	Object3dCommon::GetInstance()->SetupCommonState();


	sceneManager->Draw();
	ParticleManager::GetInstance()->Draw();

	imguiManager->End();

	DirectXCommon::GetInstance()->PostDraw();
}

void Game::Run() {
	while (true) {
		if (winApp->ProcessMessage()) {
			break;
		}

		Update();

		if (IsEndRequest()) {
			break;
		}

		Draw();
	}
}

void Game::Finalize() {
	if (imguiManager) {
		imguiManager->Finalize();
	}

	AudioManager::GetInstance()->Finalize();

	ModelManager::GetInstance()->Finalize();
	TextureManager::GetInstance()->Finalize();
	CameraManager::GetInstance()->Finalize();
	ParticleManager::GetInstance()->Finalize();

}
