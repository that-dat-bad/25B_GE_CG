#include "TitleScene.h"

#include "../engine/Debug/ImguiManager.h"
#include "../engine/Graphics/Cameramanager.h"
#include "../engine/Graphics/DirectXCommon.h"
#include "../engine/Graphics/Object3d.h"
#include "../engine/Graphics/Object3dCommon.h"
#include "../engine/Graphics/Sprite.h"
#include "../engine/Graphics/SpriteCommon.h"
#include "../engine/Graphics/TextureManager.h"
#include "../engine/Graphics/ModelManager.h"
#include "../engine/io/Input.h"

void TitleScene::Initialize(Object3dCommon *objCom, SpriteCommon *spCom,
                            DirectXCommon *dxCom, Input* input) {

  objCom_ = objCom;
  input_ = input;

  fade_.Initialize(spCom, dxCom);
  fade_.Start(Fade::Status::kFadeIn, duration_);

  phase_ = Phase::kFadeIn;

  // カメラの初期化
  // camera_.Initialize();
  CameraManager::GetInstance()->Initialize();
  CameraManager::GetInstance()->CreateCamera("default"); // 俯瞰用
  CameraManager::GetInstance()->SetActiveCamera("default");
  CameraManager::GetInstance()->GetActiveCamera()->SetTranslate(
      {0.0f, 0.0f, -90.0f});
  CameraManager::GetInstance()->GetActiveCamera()->SetRotate(
      {0.0f, 0.0f, 0.0f});

  camera_ = CameraManager::GetInstance()->GetActiveCamera();


  // 背景の初期化
  backGround_ = new BackGround();
  // 背景のモデル読み込み
  ModelManager::GetInstance()->LoadModel("models/title.obj");
  modelBackground_ = ModelManager::GetInstance()->FindModel("models/title.obj");
  objBackground_ = new Object3d();
  objBackground_->Initialize(objCom_);
  objBackground_->SetModel(modelBackground_);
  objBackground_->SetTranslate(pos);
  objBackground_->SetCamera(CameraManager::GetInstance()->GetActiveCamera());

  backGround_->Initialize(objBackground_, camera_, pos);

  // タイトルロゴの初期化
  logo_ = new TitleLogo();
  // タイトルロゴのモデル読み込み
  ModelManager::GetInstance()->LoadModel("models/titleLogo.obj");
  modelLogo_ = ModelManager::GetInstance()->FindModel("models/titleLogo.obj");
  objLogo_ = new Object3d();
  objLogo_->Initialize(objCom_);
  objLogo_->SetModel(modelLogo_);
  objLogo_->SetTranslate(pos);
  objLogo_->SetCamera(CameraManager::GetInstance()->GetActiveCamera());
  
  logo_->Initialize(objLogo_, camera_, pos);
}

void TitleScene::Update() {

    CameraManager::GetInstance()->Update();

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

  objCom_->SetupCommonState();

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
  if (input_->pushKey(DIK_SPACE)) {
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
  if (input_->pushKey(DIK_A)) {
    select_ = Select::kTutorial;
  } else if (input_->pushKey(DIK_D)) {
    select_ = Select::kGame;
  }
}
