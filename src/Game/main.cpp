#include "TDEngine.h"
#include "SceneManager.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "CameraManager.h"
#include "Math/MyMath.h" 

using namespace MyMath; 


void UpdateDebugCamera() {
	Input* input = TDEngine::GetInput();
	Camera* camera = CameraManager::GetInstance()->GetActiveCamera();

	if (!camera || !input) return;

	// 現在の情報を取得
	Vector3 rot = camera->GetRotate();
	Vector3 pos = camera->GetTranslate();

	// 回転行列を作成（移動方向の計算用）
	Matrix4x4 matRot = MakeRotateMatrix(rot);
	Vector3 cameraX = { matRot.m[0][0], matRot.m[0][1], matRot.m[0][2] }; // 右方向
	Vector3 cameraY = { matRot.m[1][0], matRot.m[1][1], matRot.m[1][2] }; // 上方向
	Vector3 cameraZ = { matRot.m[2][0], matRot.m[2][1], matRot.m[2][2] }; // 前方向

	// --- 1. 回転 (右クリック) ---
	if (input->PushMouse(1)) {
		float rotateSpeed = 0.005f;
		rot.y += input->GetMouseMoveX() * rotateSpeed; // Y軸回転 (左右)
		rot.x += input->GetMouseMoveY() * rotateSpeed; // X軸回転 (上下)
	}

	// --- 2. 平行移動 (中クリック or Shift+右クリック) ---
	if (input->PushMouse(2) || (input->PushMouse(1) && input->pushKey(DIK_LSHIFT))) {
		float panSpeed = 0.05f;
		// カメラの向きに合わせて移動
		Vector3 moveX = Multiply(-(float)input->GetMouseMoveX() * panSpeed, cameraX);
		Vector3 moveY = Multiply((float)input->GetMouseMoveY() * panSpeed, cameraY);

		pos = Add(pos, moveX);
		pos = Add(pos, moveY);
	}

	// --- 3. ズーム/前後移動 (ホイール) ---
	long wheel = input->GetWheel();
	if (wheel != 0) {
		float zoomSpeed = 0.02f;
		Vector3 moveZ = Multiply((float)wheel * zoomSpeed, cameraZ);
		pos = Add(pos, moveZ);
	}

	// 結果を反映
	camera->SetRotate(rot);
	camera->SetTranslate(pos);

	// 行列を即座に更新して反映
	camera->Update();
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	TDEngine::Initialize(L"TDEngine Game", 1280, 720);
	SceneManager* sceneManager = new SceneManager();
	sceneManager->Initialize();

	while (TDEngine::Update()) {

		ImGuiManager::GetInstance()->Begin();

		// シーン更新
		sceneManager->Update();
		UpdateDebugCamera();

		DirectXCommon::GetInstance()->PreDraw();
		sceneManager->Draw();
		ImGuiManager::GetInstance()->End();
		ImGuiManager::GetInstance()->Draw();
		DirectXCommon::GetInstance()->PostDraw();
	}

	delete sceneManager;
	TDEngine::Finalize();

	return 0;
}