#include "TitleScene.h"
#include "TDEngine.h"
#include "CameraManager.h"
#include "Input.h"

TitleScene::~TitleScene() {
	delete fade_;
	delete backGround_;
	delete logo_;
}

void TitleScene::Initialize() {
	// カメラ設定
	Camera* camera = CameraManager::GetInstance()->GetActiveCamera();
	camera->SetTranslate({ 0.0f, 0.0f, -90.0f });
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });

	// 背景
	backGround_ = new BackGround();
	backGround_->Initialize("title", { 0.0f, 0.0f, 0.0f });

	// ロゴ
	logo_ = new TitleLogo();
	logo_->Initialize({ 0.0f, 0.0f, 0.0f });

	// フェード
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::kFadeIn, duration_);
	phase_ = Phase::kFadeIn;
}

void TitleScene::Update() {
	switch (phase_) {
	case Phase::kFadeIn:  UpdateFadeIn(); break;
	case Phase::kMain:    UpdateMain();   break;
	case Phase::kFadeOut: UpdateFadeOut(); break;
	}
}

void TitleScene::Draw() {
	TDEngine::GetObject3dCommon()->SetupCommonState();
	if (backGround_) backGround_->Draw();
	if (logo_) logo_->Draw();

	TDEngine::GetSpriteCommon()->SetupCommonState();
	if (fade_) fade_->Draw();
}

void TitleScene::UpdateFadeIn() {
	fade_->Update();
	if (fade_->IsFinished()) {
		phase_ = Phase::kMain;
		fade_->Stop();
	}
	if (backGround_) backGround_->Update();
	if (logo_) logo_->Update();
}

void TitleScene::UpdateMain() {
	if (TDEngine::GetInput()->triggerKey(DIK_SPACE)) {
		if (select_ != Select::kNone) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::kFadeOut, duration_);
		}
	}
	if (backGround_) backGround_->Update();
	if (logo_) logo_->Update();
	UpdateSelect();
}

void TitleScene::UpdateFadeOut() {
	fade_->Update();
	if (fade_->IsFinished()) {
		isFinished_ = true;
	}
	if (backGround_) { backGround_->Update(); }
	if (logo_) {
		logo_->Update();
	}
}

void TitleScene::UpdateSelect() {
	// キー入力で遷移先を選択
	if (TDEngine::GetInput()->pushKey(DIK_A)) {
		select_ = Select::kTutorial;
	} else if (TDEngine::GetInput()->pushKey(DIK_D)) {
		select_ = Select::kGame;
	}
}