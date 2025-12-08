#include "ClearScene.h"
#include "TDEngine.h"
#include "CameraManager.h"

ClearScene::~ClearScene() {
	delete fade_;
	delete backGround_;
}

void ClearScene::Initialize() {
	CameraManager::GetInstance()->CreateCamera("ClearCamera");
	CameraManager::GetInstance()->SetActiveCamera("ClearCamera");
	CameraManager::GetInstance()->GetActiveCamera()->SetTranslate({ 0.0f, 0.0f, -50.0f });

	backGround_ = new BackGround();
	backGround_->Initialize("clear", { 0.0f, 0.0f, 0.0f });

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::kFadeIn, duration_);
	phase_ = Phase::kFadeIn;
}

void ClearScene::Update() {
	switch (phase_) {
	case Phase::kFadeIn:  UpdateFadeIn(); break;
	case Phase::kMain:    UpdateMain();   break;
	case Phase::kFadeOut: UpdateFadeOut(); break;
	}
}

void ClearScene::Draw() {
	TDEngine::GetObject3dCommon()->SetupCommonState();
	if (backGround_) backGround_->Draw();

	TDEngine::GetSpriteCommon()->SetupCommonState();
	if (fade_) fade_->Draw();
}

void ClearScene::UpdateFadeIn() {
	fade_->Update();
	backGround_->Update();
	if (fade_->IsFinished()) {
		phase_ = Phase::kMain;
		fade_->Stop();
	}
}

void ClearScene::UpdateMain() {
	backGround_->Update();
	if (TDEngine::GetInput()->triggerKey(DIK_SPACE)) {
		phase_ = Phase::kFadeOut;
		fade_->Start(Fade::Status::kFadeOut, duration_);
	}
}

void ClearScene::UpdateFadeOut() {
	fade_->Update();
	backGround_->Update();
	if (fade_->IsFinished()) {
		isFinished_ = true;
	}
}