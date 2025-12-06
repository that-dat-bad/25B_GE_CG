#include "TitleScene.h"
#include "../External/TDEngine/include/2d/ImGuiManager.h"

using namespace TDEngine;

void TitleScene::Initialize() {
	fade_.Initialize();
	fade_.Start(Fade::Status::kFadeIn, duration_);

	phase_ = Phase::kFadeIn;

	// カメラの初期化
	camera_.Initialize();

	// 背景の初期化
	backGround_ = new BackGround();
	// 背景のモデル読み込み
	modelBackground_ = Model::CreateFromOBJ("title", true);
	backGround_->Initialize(modelBackground_, &camera_, pos);
	// タイトルロゴの初期化
	logo_ = new TitleLogo();
	// タイトルロゴのモデル読み込み
	modelLogo_ = Model::CreateFromOBJ("titleLogo", true);
	logo_->Initialize(modelLogo_, &camera_, pos);
	
}

void TitleScene::Update() {

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

	ImGui::Begin("TitleScene");

	ImGui::Text("Select: %d", static_cast<int>(select_));

	ImGui::End();
}

void TitleScene::Draw() { 

	Model::PreDraw();

	backGround_->Draw();
	logo_->Draw();

	Model::PostDraw();

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
	if (Input::GetInstance()->PushKey(DIK_SPACE)) {
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
	if (Input::GetInstance()->PushKey(DIK_A)) {
		select_ = Select::kTutorial;
	} else if (Input::GetInstance()->PushKey(DIK_D)) {
		select_ = Select::kGame;
	}
}
