#include "TutorialScene.h"
#include "CameraManager.h"
#include "TDEngine.h"
#include "TextureManager.h"

TutorialScene::~TutorialScene() {
  delete fade_;
  delete player_;
  delete enemy_;
  delete skydome_;
  delete rule_;
  delete operation_;
  AudioManager* audio = TDEngine::GetAudioManager();
  audio->StopAllVoices();
  pBgmVoice_ = nullptr;
  audio->SoundUnload(&soundBgm_);
}

void TutorialScene::Initialize() {
  // カメラ初期化 (CameraManagerを利用)
  // "GameCamera" という名前で作成し、アクティブにする
  CameraManager::GetInstance()->CreateCamera("TutorialCamera");
  CameraManager::GetInstance()->SetActiveCamera("TutorialCamera");

  // カメラの位置設定 (TDEngineのCameraクラスの仕様に合わせる)
  Camera *camera = CameraManager::GetInstance()->GetActiveCamera();
  camera->SetTranslate({0.0f, 0.0f, -50.0f}); // 適切な距離に設定
  camera->SetRotate({0.0f, 0.0f, 0.0f});

  skydome_ = new Skydome();
  skydome_->Initialize();

  player_ = new Player();
  player_->Initialize({-27.0f, 0.0f, 0.0f});

  enemy_ = new Enemy();
  enemy_->Initialize({0.0f, 20.0f, 0.0f});
  enemy_->SetPlayer(player_);
  enemy_->SetRequest(true);

  fade_ = new Fade();
  fade_->Initialize();
  fade_->Start(Fade::Status::kFadeIn, duration_);
  phase_ = Phase::kFadeIn;

  TextureManager::LoadTexture("./Resources/rule.png");
  // スプライト生成
  rule_ = Sprite::Create("./Resources/rule.png", {640.0f, 70.0f}, {1, 1, 1, 1},
                         {0.5f, 0.5f});

  TextureManager::LoadTexture("./Resources/operation.png");
  // スプライト生成
  operation_ = Sprite::Create("./Resources/operation.png", {250.0f, 670.0f},
                             {1, 1, 1, 1},
                         {0.5f, 0.5f});
  operation_->SetScale(Vector2{0.5f, 0.5f});

  // BGM
  AudioManager* audio = TDEngine::GetAudioManager();
  soundBgm_ = audio->SoundLoadWave("Resources/Sound/tutorial.wav");
  pBgmVoice_ = audio->SoundPlayWave(soundBgm_, true, 1.0f);
}

void TutorialScene::Update() {

  if (skydome_) {
    skydome_->Update();
  }
  if (player_) {
    player_->Update();
  }

  if (enemy_) {
    enemy_->Update();
  }

  rule_->Update();
  operation_->Update();

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

void TutorialScene::Draw() {
  TDEngine::GetObject3dCommon()->SetupCommonState();
  if (skydome_) {
    skydome_->Draw();
  }
  if (player_) {
    player_->Draw();
  }
  if (enemy_) {
    enemy_->Draw();
  }


  TDEngine::GetSpriteCommon()->SetupCommonState();

  rule_->Draw(TDEngine::GetSpriteCommon()->GetDirectXCommon());
  operation_->Draw(TDEngine::GetSpriteCommon()->GetDirectXCommon());

  if (fade_) {
    fade_->Draw();
  }
}

void TutorialScene::UpdateFadeIn() {
  fade_->Update();

  if (fade_->IsFinished()) {
    phase_ = Phase::kMain;
    fade_->Stop();
  }
}

void TutorialScene::UpdateMain() {

  if (isCleared_) {
    changeTimer_--;
    if (changeTimer_ <= 0) {
      phase_ = Phase::kFadeOut;
      fade_->Start(Fade::Status::kFadeOut, duration_);
    }
  } else {
    CheckAllCollisions();
  }
}

void TutorialScene::CheckAllCollisions() {
  if (!player_ || !enemy_)
    return;
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
  if (skydome_)
    skydome_->Update();
  if (fade_->IsFinished()) {
    isFinished_ = true;
  }
}