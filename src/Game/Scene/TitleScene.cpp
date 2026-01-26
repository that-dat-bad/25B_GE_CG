#include "TitleScene.h"
#include <assert.h>
#include <cmath>
#include "Title/TitleLogo.h"
#include "Object3dCommon.h"
#include "TextureManager.h"
#include "ImguiManager.h"

using namespace MyMath;

TitleScene::~TitleScene() {
	delete debugCamera_;
	delete logo_;
	// unique_ptrは自動解放
}

void TitleScene::Finalize() {
	// 必要ならモデル解放など
}

void TitleScene::Initialize() {
	camera_.Initialize();

	// カメラの位置を -50 に設定
	camera_.translation_.z = -50.0f;

	// Inputインスタンスの取得
	input_ = Input::GetInstance();

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// --- フェーズとタイマーの初期化 ---
	phase_ = ScenePhase::kFadeIn;
	fadeTimer_ = kFadeDuration_;

	// --- フェード用スプライトの初期化 ---
	TextureManager::GetInstance()->LoadTexture("white1x1.png");
	fadeTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("white1x1.png");

	// --- 3Dモデル (タイトルロゴ) の初期化 ---
	logoPosition_ = { 0.0f, 0.0f, -45.0f };

	// モデルのロード (存在確認してからロード)
	ModelManager::GetInstance()->LoadModel("title");

	logo_ = new TitleLogo();
	// モデルマネージャからポインタを取得して渡す
	Model* logoModel = ModelManager::GetInstance()->FindModel("title");
	logo_->Initialize(logoModel, &camera_, logoPosition_);
	logo_->SetPosition(logoPosition_);

	// --- 背景 (天球) の初期化 ---
	// モデルロード
	ModelManager::GetInstance()->LoadModel("titleSkydome");
	skydomeModelResource_ = ModelManager::GetInstance()->FindModel("titleSkydome");
	
	// Object3d生成
	skydomeObject_ = std::make_unique<Object3d>();
	skydomeObject_->Initialize(Object3dCommon::GetInstance());
	skydomeObject_->SetModel(skydomeModelResource_);
	skydomeObject_->SetCamera(&camera_); // カメラセット
	
	// 天球なので大きく
	skydomeObject_->SetScale({ 100.0f, 100.0f, 100.0f });
	
	// 更新
	skydomeObject_->Update();

	// --- 誘導表示 (Press Space スプライト) の初期化 ---
	TextureManager::GetInstance()->LoadTexture("./Resources/pressSpace.png");
	pressSpaceTexture_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("./Resources/pressSpace.png");

	// スプライト生成
	pressSpaceSprite_.reset(new Sprite(
		pressSpaceTexture_, { 640.0f, 550.0f }, { 1280.0f, 130.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.5f, 0.5f }, // アンカーポイントを中央に
		false, false));
	pressSpaceSprite_->Initialize();
	blinkTimer_ = 0.0f;

	// --- フェードスプライト設定 ---
	Vector2 position = { 0.0f, 0.0f };
	Vector2 size = { 1280.0f, 720.0f };

	// フェード色を「白」に設定
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector2 anchorpoint = { 0.0f, 0.0f };

	fadeSprite_.reset(new Sprite(fadeTextureHandle_, position, size, color, anchorpoint, false, false));
	fadeSprite_->Initialize();
	fadeSprite_->SetTextureRect({ 0.0f, 0.0f }, { 1.0f, 1.0f });
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

	// --- 共通更新 ---
	logo_->SetPosition(logoPosition_);
	logo_->Update();

	// カメラの更新
	// カメラの更新
	if (!isDebugCameraActive_) {
		camera_.Update();
	}
	
	// 天球更新
	if(skydomeObject_){
		skydomeObject_->Update();
	}

	return result;
}

void TitleScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// ==========================================
	// 3Dモデル描画フェーズ
	// ==========================================
	Model::PreDraw(dxCommon->GetCommandList());

	// 1. 背景 (天球)
	if(skydomeObject_){
		skydomeObject_->Draw();
	}

	// 2. タイトルロゴ
	logo_->Draw();

	Model::PostDraw();

	// ==========================================
	// スプライト描画フェーズ (2D)
	// ==========================================
	Sprite::PreDraw(dxCommon->GetCommandList(), Sprite::BlendMode::kNormal);

	// 1. Press Space 誘導表示
	if (pressSpaceSprite_) {
		pressSpaceSprite_->Draw();
	}

	// 2. フェード用画像 (最前面)
	float alpha = 0.0f;
	if (phase_ == ScenePhase::kFadeIn) {
		alpha = (float)fadeTimer_ / (float)kFadeDuration_;
	} else if (phase_ == ScenePhase::kFadeOut) {
		alpha = (float)fadeTimer_ / (float)kFadeDuration_;
	}

	if (alpha > 0.0f && fadeSprite_) {
		// 白で描画
		fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha });
		fadeSprite_->Draw();
	}

	Sprite::PostDraw();
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
	

	// --- スプライトの点滅処理 ---
	blinkTimer_ += 0.1f; // 点滅スピード
	// サイン波でアルファ値を 0.0 ～ 1.0 にする
	float textAlpha = (std::sin(blinkTimer_) + 1.0f) / 2.0f;
	pressSpaceSprite_->SetColor({ 1.0f, 1.0f, 1.0f, textAlpha });

	// スペースキーで次へ
	if (input_->TriggerKey(DIK_SPACE)) {
		phase_ = ScenePhase::kFadeOut;
		fadeTimer_ = 0;
	}

	return std::nullopt;
}

std::optional<SceneID> TitleScene::UpdateFadeOut() {
	fadeTimer_++;

	// ロゴをカメラ方向（マイナス方向）へ移動
	logoPosition_.z -= 0.5f;

	if (logoPosition_.z <= -49.99f) {
		logoPosition_.z = -49.99f;
	}

	logo_->SetPosition(logoPosition_);
	logo_->Update();

	if (fadeTimer_ >= kFadeDuration_) {
		return SceneID::kStage; // GameではなくStageに変更
	}
	return std::nullopt;
}