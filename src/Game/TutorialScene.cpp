#include "TutorialScene.h"
#include "TDEngine.h"
#include "CameraManager.h"

TutorialScene::~TutorialScene() {
	delete fade_;
	delete player_;
	delete enemy_;
	delete skydome_;
}

void TutorialScene::Initialize() {
	CameraManager::GetInstance()->GetActiveCamera()->SetTranslate({ 0.0f, 0.0f, -90.0f });

	skydome_ = new Skydome();
	skydome_->Initialize();

	player_ = new Player();
	player_->Initialize({ -27.0f, 0.0f, 0.0f });

	enemy_ = new Enemy();
	enemy_->Initialize({ 0.0f, 20.0f, 0.0f });
	enemy_->SetPlayer(player_);
	enemy_->SetRequest(true);

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::kFadeIn, duration_);
	phase_ = Phase::kFadeIn;
}

void TutorialScene::Update() {
	switch (phase_) {
	case Phase::kFadeIn:  UpdateFadeIn(); break;
	case Phase::kMain:    UpdateMain();   break;
	case Phase::kFadeOut: UpdateFadeOut(); break;
	}
}

void TutorialScene::Draw() {
	TDEngine::GetObject3dCommon()->SetupCommonState();
	if (skydome_) skydome_->Draw();
	if (player_) player_->Draw();
	if (enemy_) enemy_->Draw();

	TDEngine::GetSpriteCommon()->SetupCommonState();
	if (fade_) fade_->Draw();
}

void TutorialScene::UpdateFadeIn() {
	fade_->Update();
	if (skydome_) skydome_->Update();
	if (fade_->IsFinished()) {
		phase_ = Phase::kMain;
		fade_->Stop();
	}
}

void TutorialScene::UpdateMain() {
	if (skydome_) skydome_->Update();
	if (player_) player_->Update();

	if (isCleared_) {
		changeTimer_--;
		if (changeTimer_ <= 0) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::kFadeOut, duration_);
		}
	}
	else {
		if (enemy_) enemy_->Update();
		CheckAllCollisions();
	}
}

void TutorialScene::CheckAllCollisions() {
	if (!player_ || !enemy_) return;
	AABB playerAABB = player_->GetAABB();
	AABB enemyAABB = enemy_->GetAABB();

	if (Collision::IsCollision(playerAABB, enemyAABB)) {
		player_->OnCollision(enemy_);
		enemy_->OnCollision(player_);

		if (player_->IsDead() && enemy_->IsCollisionDisabled()) {
			isCleared_ = true;
		}
	}
}

void TutorialScene::UpdateFadeOut() {
	fade_->Update();
	if (skydome_) skydome_->Update();
	if (fade_->IsFinished()) {
		isFinished_ = true;
	}
}