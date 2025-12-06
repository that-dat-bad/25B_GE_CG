#include "GameOverScene.h"
using namespace TDEngine;


void GameOverScene::Initialize() {
	fade_.Initialize();
	fade_.Start(Fade::Status::kFadeIn, duration_);

	phase_ = Phase::kFadeIn;

	// カメラの初期化
	camera_.Initialize();
	// 背景の初期化
	backGround_ = new BackGround();
	// 背景のモデル読み込み
	modelBackground_ = Model::CreateFromOBJ("gameover", true);
	backGround_->Initialize(modelBackground_, &camera_, pos);
}

void GameOverScene::Update() {
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
}

void GameOverScene::Draw() {
	Model::PreDraw();

	backGround_->Draw();

	Model::PostDraw();

	fade_.Draw();
}

void GameOverScene::UpdateFadeIn() {
	fade_.Update();
	backGround_->Update();

	if (fade_.IsFinished()) {
		phase_ = Phase::kMain;
		fade_.Stop();
	}
}

void GameOverScene::UpdateMain() {
	backGround_->Update();

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		phase_ = Phase::kFadeOut;
		fade_.Start(Fade::Status::kFadeOut, duration_);
	}
}

void GameOverScene::UpdateFadeOut() {
	fade_.Update();
	backGround_->Update();

	if (fade_.IsFinished()) {
		isFinished_ = true;
	}
}
