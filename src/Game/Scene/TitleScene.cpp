#include "TitleScene.h"
#include "Input.h"
#include "DebugCamera.h"
#include <assert.h>
#include <cmath>
#include "Title/TitleLogo.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ImguiManager.h"

using namespace MyMath;

TitleScene::~TitleScene() {
	delete debugCamera_;
	delete logo_;
}

void TitleScene::Finalize() {
}

void TitleScene::Initialize() {
	camera_.SetTranslate({ 0.0f, 0.0f, -50.0f });

	input_ = Input::GetInstance();

	debugCamera_ = new DebugCamera();

	phase_ = ScenePhase::kFadeIn;
	fadeTimer_ = kFadeDuration_;

	TextureManager::GetInstance()->LoadTexture("white1x1.png");
	fadeTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("white1x1.png");

	logoPosition_ = { 0.0f, 0.0f, -45.0f };

	ModelManager::GetInstance()->LoadModel("title");

	logo_ = new TitleLogo();
	Model* logoModel = ModelManager::GetInstance()->FindModel("title");
	logo_->Initialize(logoModel, &camera_, logoPosition_);
	logo_->SetPosition(logoPosition_);

	ModelManager::GetInstance()->LoadModel("titleSkydome");
	skydomeModelResource_ = ModelManager::GetInstance()->FindModel("titleSkydome");
	
	skydomeObject_ = std::make_unique<Object3d>();
	skydomeObject_->Initialize(Object3dCommon::GetInstance());
	skydomeObject_->SetModel(skydomeModelResource_);
	skydomeObject_->SetCamera(&camera_); 
	
	skydomeObject_->SetScale({ 100.0f, 100.0f, 100.0f });
	
	skydomeObject_->Update();

	TextureManager::GetInstance()->LoadTexture("./Resources/pressSpace.png");
	pressSpaceTexture_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("./Resources/pressSpace.png");

	SpriteCommon* spriteCommon = SpriteCommon::GetInstance();
	pressSpaceSprite_.reset(new Sprite());
	pressSpaceSprite_->Initialize(spriteCommon, "./Resources/pressSpace.png");
	pressSpaceSprite_->SetPosition({ 640.0f, 550.0f });
	pressSpaceSprite_->SetSize({ 1280.0f, 130.0f });
	pressSpaceSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	pressSpaceSprite_->SetAnchorPoint({ 0.5f, 0.5f });
	blinkTimer_ = 0.0f;

	fadeSprite_.reset(new Sprite());
	fadeSprite_->Initialize(spriteCommon, "white1x1.png");
	fadeSprite_->SetPosition({ 0.0f, 0.0f });
	fadeSprite_->SetSize({ 1280.0f, 720.0f });
	fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	fadeSprite_->SetAnchorPoint({ 0.0f, 0.0f });
	fadeSprite_->SetTextureLeftTop({ 0.0f, 0.0f });
	fadeSprite_->SetTextureSize({ 1.0f, 1.0f });
}

std::optional<SceneID> TitleScene::Update() {
	std::optional<SceneID> result = std::nullopt;

	switch (phase_) {
	case ScenePhase::kFadeIn:
		result = UpdateFadeIn();
		break;
	case ScenePhase::kMain:
		result = UpdateMain();
		break;
	case ScenePhase::kFadeOut:
		result = UpdateFadeOut();
		break;
	}

	logo_->SetPosition(logoPosition_);
	logo_->Update();

	if (!isDebugCameraActive_) {
		camera_.Update();
	}
	
	if(skydomeObject_){
		skydomeObject_->Update();
	}

	return result;
}

void TitleScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// ==========================================
	// ==========================================
	Object3dCommon::GetInstance()->SetupCommonState();

	if(skydomeObject_){
		skydomeObject_->Draw();
	}

	logo_->Draw();


	// ==========================================
	// ==========================================
	SpriteCommon::GetInstance()->SetupCommonState();

	if (pressSpaceSprite_) {
		pressSpaceSprite_->Update();
		pressSpaceSprite_->Draw();
	}

	float alpha = 0.0f;
	if (phase_ == ScenePhase::kFadeIn) {
		alpha = (float)fadeTimer_ / (float)kFadeDuration_;
	} else if (phase_ == ScenePhase::kFadeOut) {
		alpha = (float)fadeTimer_ / (float)kFadeDuration_;
	}

	if (alpha > 0.0f && fadeSprite_) {
		fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha });
		fadeSprite_->Update();
		fadeSprite_->Draw();
	}

}

std::optional<SceneID> TitleScene::UpdateFadeIn() {
	fadeTimer_--;
	if (fadeTimer_ <= 0) {
		phase_ = ScenePhase::kMain;
	}
	return std::nullopt;
}

std::optional<SceneID> TitleScene::UpdateMain() {
#ifdef USE_IMGUI
	if (input_->TriggerKey(DIK_0)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
	if (isDebugCameraActive_) {
		ImGui::Begin("Debug Camera");
		ImGui::Text("Debug Camera: ON");
		ImGui::End();
	}
	ImGui::Begin("Title Adjustment");
	ImGui::DragFloat3("Logo Position", &logoPosition_.x, 0.1f);
	ImGui::End();
#endif



		camera_.Update();
	

	blinkTimer_ += 0.1f; 
	float textAlpha = (std::sin(blinkTimer_) + 1.0f) / 2.0f;
	pressSpaceSprite_->SetColor({ 1.0f, 1.0f, 1.0f, textAlpha });

	if (input_->TriggerKey(DIK_SPACE)) {
		phase_ = ScenePhase::kFadeOut;
		fadeTimer_ = 0;
	}

	return std::nullopt;
}

std::optional<SceneID> TitleScene::UpdateFadeOut() {
	fadeTimer_++;

	logoPosition_.z -= 0.5f;

	if (logoPosition_.z <= -49.99f) {
		logoPosition_.z = -49.99f;
	}

	logo_->SetPosition(logoPosition_);
	logo_->Update();

	if (fadeTimer_ >= kFadeDuration_) {
		return SceneID::kStage; 
	}
	return std::nullopt;
}
