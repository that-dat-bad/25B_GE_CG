#include "GameOverScene.h"
#include "TDEngine.h"
#include "CameraManager.h"

GameOverScene::~GameOverScene() {
	delete fade_;
	delete backGround_;
	AudioManager* audio = TDEngine::GetAudioManager();
	//audio->StopAllVoices();
	pBgmVoice_ = nullptr;
	audio->SoundUnload(&soundBgm_);
}

void GameOverScene::Initialize() {
	CameraManager::GetInstance()->CreateCamera("GameOverCamera");
	CameraManager::GetInstance()->SetActiveCamera("GameOverCamera");
	CameraManager::GetInstance()->GetActiveCamera()->SetTranslate({ 0.0f, 0.0f, -50.0f });

	backGround_ = new BackGround();
	backGround_->Initialize("gameover", { 0.0f, 0.0f, 0.0f });

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::kFadeIn, duration_);
	phase_ = Phase::kFadeIn;

	// BGM
	AudioManager* audio = TDEngine::GetAudioManager();
	soundBgm_ = audio->SoundLoadWave("Resources/Sound/gameover.wav");
	pBgmVoice_ = audio->SoundPlayWave(soundBgm_, false, 1.0f);
}

void GameOverScene::Update() {
	switch (phase_) {
	case Phase::kFadeIn:  UpdateFadeIn(); break;
	case Phase::kMain:    UpdateMain();   break;
	case Phase::kFadeOut: UpdateFadeOut(); break;
	}
}

void GameOverScene::Draw() {
	TDEngine::GetObject3dCommon()->SetupCommonState();
	if (backGround_) backGround_->Draw();

	TDEngine::GetSpriteCommon()->SetupCommonState();
	if (fade_) fade_->Draw();
}

void GameOverScene::UpdateFadeIn() {
	fade_->Update();
	backGround_->Update();
	if (fade_->IsFinished()) {
		phase_ = Phase::kMain;
		fade_->Stop();
	}
}

void GameOverScene::UpdateMain() {
	backGround_->Update();
	if (TDEngine::GetInput()->triggerKey(DIK_SPACE)) {
		phase_ = Phase::kFadeOut;
		fade_->Start(Fade::Status::kFadeOut, duration_);
	}
}

void GameOverScene::UpdateFadeOut() {
	fade_->Update();
	backGround_->Update();
	if (fade_->IsFinished()) {
		isFinished_ = true;
		AudioManager* audio = TDEngine::GetAudioManager();
		audio->StopVoice(pBgmVoice_);
	}
}