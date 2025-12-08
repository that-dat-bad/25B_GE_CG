#include "TDEngine.h"

using namespace TDEngine;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	TDEngine::Initialize(L"CG2");


	Model::LoadFromOBJ("assets/models/axis.obj"); 
	Model::LoadFromOBJ("assets/models/plane.obj");

	Object3d* object3d = Object3d::Create();
	object3d->SetModel("assets/models/axis.obj"); // 読み込んだモデルをセット
	object3d->SetTranslate({ -2.0f, 0.0f, 0.0f });

	Object3d* plane = Object3d::Create();
	plane->SetModel("assets/models/plane.obj");

	
	// メインループ
	while (true) {
		if (TDEngine::Update()) break;
		
		ImGuiManager::GetInstance()->Begin();
		// ... Update処理 ...
		object3d->Update();
		plane->Update();

		ImGuiManager::GetInstance()->End();

		DirectXCommon::GetInstance()->PreDraw();

		// ... 描画 ...
		object3d->Draw();
		plane->Draw();

		ImGuiManager::GetInstance()->Draw();
		DirectXCommon::GetInstance()->PostDraw();
	}

	// 解放
	delete object3d;
	delete plane;
	TDEngine::Finalize();

	return 0;
}