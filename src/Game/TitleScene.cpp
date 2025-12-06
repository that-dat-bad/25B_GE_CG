#include "TitleScene.h"
// #include "../External/KamataEngine/include/2d/ImGuiManager.h" // 不要

using namespace TDEngine; // KamataEngine -> TDEngine

void TitleScene::Initialize() {
	fade_.Initialize();
	fade_.Start(Fade::Status::kFadeIn, duration_);

	phase_ = Phase::kFadeIn;


	// カメラの更新を一回呼んで行列を作っておく
	camera_.Update();

	// 背景の初期化
	backGround_ = new BackGround();
	// 背景のモデル読み込み
	modelBackground_ = Model::CreateFromOBJ("title", true);

	// カメラを渡す
	backGround_->Initialize(modelBackground_, &camera_, pos);

	// タイトルロゴの初期化
	logo_ = new TitleLogo();
	// タイトルロゴのモデル読み込み
	modelLogo_ = Model::CreateFromOBJ("titleLogo", true);
	logo_->Initialize(modelLogo_, &camera_, pos);
}

void TitleScene::Update() {

	// シーンごとのカメラ更新が必要ならここで呼ぶ
	camera_.Update();

	switch (phase_) {
	case Phase::kFadeIn:
		UpdateFadeIn();
		break;
	case Phase::kMain:
		UpdateMain();
		break;
	case Phase::kFadeOut:
		UpdateFadeOut();
		break;
	}

	ImGuiManager::GetInstance()->Begin(); // TDEngineでは必要なら
	ImGui::Begin("TitleScene");
	ImGui::Text("Select: %d", static_cast<int>(select_));
	ImGui::End();
	ImGuiManager::GetInstance()->End();
}

void TitleScene::Draw() {


	backGround_->Draw();
	logo_->Draw();


	fade_.Draw();

}

void TitleScene::UpdateFadeIn() {
	fade_.Update();

	if (fade_.IsFinished()) {
		phase_ = Phase::kMain;
		fade_.Stop();
	}

	// 背景の更新
	backGround_->Update();
	logo_->Update();
}

void TitleScene::UpdateMain() {
	// Input::GetInstance() -> TDEngine::GetInput()
	if (GetInput()->PushKey(DIK_SPACE)) {
		if (select_ != Select::kNone) {
			phase_ = Phase::kFadeOut;
			fade_.Start(Fade::Status::kFadeOut, duration_);
		}
	}

	// 背景の更新
	backGround_->Update();
	logo_->Update();

	UpdateSelect();
}

void TitleScene::UpdateFadeOut() {
	fade_.Update();

	if (fade_.IsFinished()) {
		isFinished_ = true;
	}

	// 背景の更新
	backGround_->Update();
	logo_->Update();
}

/// <summary>
/// 選択
/// </summary>
void TitleScene::UpdateSelect() {
	if (GetInput()->PushKey(DIK_A)) {
		select_ = Select::kTutorial;
	}
	else if (GetInput()->PushKey(DIK_D)) {
		select_ = Select::kGame;
	}
}