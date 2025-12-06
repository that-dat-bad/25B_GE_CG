#include "TutorialScene.h"
// TDEngine.h に ImGuiManager が含まれているため、個別インクルードは不要ですが
// 必要に応じてパスを調整してください。
// #include "../External/TDEngine/include/2d/ImGuiManager.h" 

using namespace TDEngine;

void TutorialScene::Initialize() {
	fade_.Initialize();
	fade_.Start(Fade::Status::kFadeIn, duration_);

	phase_ = Phase::kFadeIn;

	// カメラの更新 (Initializeは無いのでUpdateで行列作成)
	camera_.Update();

	// 天球の生成
	skydome_ = new Skydome();

	// 天球の3Dモデルの生成
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);

	// 天球の初期化
	skydome_->Initialize(modelSkydome_, &camera_);

	// プレイヤー
	player_ = new Player();
	// プレイヤーの3Dモデルの生成
	modelPlayer_ = Model::CreateFromOBJ("player", true);
	// プレイヤーの初期化
	player_->Initialize(modelPlayer_, &camera_, playerPos_);

	// 敵の生成
	enemy_ = new Enemy();
	// 敵の3Dモデルの生成
	modelEnemy_ = Model::CreateFromOBJ("enemy", true);
	// 敵の初期化
	enemy_->Initialize(modelEnemy_, &camera_, enemyPos_);
	// プレイヤーのポインタを敵にセット
	enemy_->SetPlayer(player_);
	// 敵の振る舞いリクエストを決定
	enemy_->SetRequest(true);
}

void TutorialScene::Update() {
	// カメラの更新
	camera_.Update();

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

	// ImGui
	ImGuiManager::GetInstance()->Begin(); // TDEngineの仕組みに合わせて呼ぶ
	ImGui::Begin("TutorialScene");

	ImGui::Text("push Enter: isCleared_= %s", isCleared_ ? "true" : "false");
	if (isCleared_) {
		ImGui::Text("push Space: isFinished_= %s", isFinished_ ? "true" : "false");
	}

	ImGui::End();
	ImGuiManager::GetInstance()->End();
}

void TutorialScene::Draw() {

	// Model::PreDraw(); // 不要

	// 天球の描画
	skydome_->Draw();

	// プレイヤーの描画
	player_->Draw();
	// 敵の描画
	enemy_->Draw();

	// Model::PostDraw(); // 不要

	fade_.Draw();
}

// フェードインの更新
void TutorialScene::UpdateFadeIn() {
	fade_.Update();
	skydome_->Update();

	if (fade_.IsFinished()) {
		phase_ = Phase::kMain;
		fade_.Stop();
	}
}
// メインの更新
void TutorialScene::UpdateMain() {

	skydome_->Update();

	// プレイヤーの更新処理
	player_->Update();

	if (isCleared_) {
		changeTimer_--;

		if (changeTimer_ <= 0) {
			phase_ = Phase::kFadeOut;
			fade_.Start(Fade::Status::kFadeOut, duration_);
		}

	}
	else {

		// 敵の更新処理
		enemy_->Update();

		// 当たり判定
		CheckAllCollisions();
	}
}

void TutorialScene::CheckAllCollisions() {
	// プレイヤー
	AABB aabbPlayer;
	// プレイヤーの座標
	aabbPlayer = player_->GetAABB();

	// 敵
	AABB aabbEnemy;
	// 敵の座標
	aabbEnemy = enemy_->GetAABB();

	if (IsCollision(aabbPlayer, aabbEnemy)) {
		// プレイヤーの衝突関数
		player_->OnCollision(enemy_);
		// 敵の衝突関数
		enemy_->OnCollision(player_);

		// 爆発したかどうかを取得
		if (player_->IsDead() && enemy_->IsCollisionDisabled()) {
			isCleared_ = true;
		}
	}
}

bool TutorialScene::IsCollision(const AABB& aabb1, const AABB& aabb2) {
	if (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x && aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y && aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z) {
		return true;
	}

	return false;
}

// フェードアウトの更新
void TutorialScene::UpdateFadeOut() {
	fade_.Update();
	skydome_->Update();

	if (fade_.IsFinished()) {
		isFinished_ = true;
	}
}