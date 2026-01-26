#include "TitleScene.h"
#include <assert.h>
#include <cmath>


TitleScene::~TitleScene() {}

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
	fadeTextureHandle_ = TextureManager::Load("white1x1.png");

	// --- 3Dモデル (タイトルロゴ) の初期化 ---
	logoPosition_ = { 0.0f, 0.0f, -45.0f };

	logo_ = new TitleLogo();
	logo_->Initialize(Model::CreateFromOBJ("title"), &camera_, logoPosition_);
	logo_->SetPosition(logoPosition_);

	// --- 背景 (天球) の初期化 ---
	skydomeModel_ = Model::CreateFromOBJ("titleSkydome");
	skydomeTransform_.Initialize();
	// 行列更新
	skydomeTransform_.matWorld_ = MakeAffineMatrix(skydomeTransform_.scale_, skydomeTransform_.rotation_, skydomeTransform_.translation_);
	skydomeTransform_.TransferMatrix();

	// --- 誘導表示 (Press Space スプライト) の初期化 ---
	pressSpaceTexture_ = TextureManager::Load("./Resources/pressSpace.png");

	// スプライト生成
	pressSpaceSprite_ = new Sprite(
		pressSpaceTexture_, { 640.0f, 550.0f }, { 1280.0f, 130.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.5f, 0.5f }, // アンカーポイントを中央に
		false, false);
	pressSpaceSprite_->Initialize();
	blinkTimer_ = 0.0f;

	// --- フェードスプライト設定 ---
	Vector2 position = { 0.0f, 0.0f };
	Vector2 size = { 1280.0f, 720.0f };

	// フェード色を「白」に設定
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

	Vector2 anchorpoint = { 0.0f, 0.0f };

	fadeSprite_ = new Sprite(fadeTextureHandle_, position, size, color, anchorpoint, false, false);
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
	if (!isDebugCameraActive_) {
		camera_.UpdateMatrix();
		camera_.TransferMatrix();
	}

	return result;
}

void TitleScene::Draw() {
	KamataEngine::DirectXCommon* dxCommon = KamataEngine::DirectXCommon::GetInstance();

	// ==========================================
	// 3Dモデル描画フェーズ
	// ==========================================
	KamataEngine::Model::PreDraw(dxCommon->GetCommandList());

	// 1. 背景 (天球)

	skydomeModel_->Draw(skydomeTransform_, camera_);

	// 2. タイトルロゴ
	logo_->Draw();

	KamataEngine::Model::PostDraw();

	// ==========================================
	// スプライト描画フェーズ (2D)
	// ==========================================
	KamataEngine::Sprite::PreDraw(dxCommon->GetCommandList(), KamataEngine::Sprite::BlendMode::kNormal);

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

	KamataEngine::Sprite::PostDraw();
}

std::optional<SceneID> TitleScene::UpdateFadeIn() {
	fadeTimer_--;
	if (fadeTimer_ <= 0) {
		phase_ = ScenePhase::kMain;
	}
	return std::nullopt;
}

std::optional<SceneID> TitleScene::UpdateMain() {
#ifdef _DEBUG
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

	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		camera_.UpdateMatrix();
		camera_.TransferMatrix();
	}

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
		return SceneID::kGame;
	}
	return std::nullopt;
}