#define NOMINMAX
#include "GameScene.h"
// #include "../External/TDEngine/include/2d/ImGuiManager.h" 
#include "ChainBomb.h"
#include "Player.h"
#include "TimeLimit.h"
#include "Wind.h"

#include <algorithm>
#include <limits>

using namespace TDEngine;
using namespace MyMath; // ★追加: Dot関数などを使うために必要

const char* ToString(TimeLimit::Phase phase) {
	switch (phase) {
	case TimeLimit::Phase::kStartCountDown:
		return "StartCountDown";
	case TimeLimit::Phase::kActive:
		return "Active";
	case TimeLimit::Phase::kLast5Second:
		return "Last5Second";
	case TimeLimit::Phase::kTimeUp:
		return "TimeUp";
	}
	return "Unknown";
}

// デストラクタ
GameScene::~GameScene() {
	delete player_;
	delete modelPlayer_;
	delete enemy_;
	delete modelEnemy_;

	for (ChainBomb* chainBomb : chainBombs_) {
		delete chainBomb;
	}
	chainBombs_.clear();
	delete modelChainBomb_;

	delete wind_;
	delete modelWind_;
}

// 初期化処理
void GameScene::Initialize() {

	// カメラの初期化
	camera_.Update();

	// 天球の生成
	skydome_ = new Skydome();
	modelSkydome_ = TDEngine::Model::CreateFromOBJ("skydome", true);
	skydome_->Initialize(modelSkydome_, &camera_);


	// プレイヤー
	player_ = new Player();
	modelPlayer_ = TDEngine::Model::CreateFromOBJ("player", true);
	player_->Initialize(modelPlayer_, &camera_, playerPos_);

	// 敵の生成
	enemy_ = new Enemy();
	modelEnemy_ = TDEngine::Model::CreateFromOBJ("enemy", true);
	enemy_->Initialize(modelEnemy_, &camera_, enemyPos_);
	enemy_->SetPlayer(player_);

	// 連鎖ボム
	modelChainBomb_ = TDEngine::Model::CreateFromOBJ("bomb", true);
	for (int32_t i = 0; i < 5; ++i) {
		ChainBomb* chainBomb = new ChainBomb();
		chainBombPos_ = { -5.0f + i * 5.0f, 0.0f + i * 1.3f, 0 };
		chainBomb->Initialize(modelChainBomb_, &camera_, chainBombPos_);
		chainBombs_.push_back(chainBomb);
	}

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	// 風の生成
	wind_ = new Wind();
	modelWind_ = TDEngine::Model::CreateFromOBJ("wind", true);
	wind_->Initialize(modelWind_, &camera_, windPos_);

	// 時間制限
	timeLimit_ = new TimeLimit();
	timeLimit_->Initialize();

	fade_.Initialize();
	fade_.Start(Fade::Status::kFadeIn, duration_);
	phase_ = Phase::kFadeIn;
}

// 更新処理
void GameScene::Update() {

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

	ImGuiManager::GetInstance()->Begin();
	ImGui::Begin("GameScene");

	ImGui::Text("Phase      : %s", ToString(timeLimit_->phase_));
	ImGui::Text("Timer      : %.2f [sec]", timeLimit_->timer_);
	ImGui::Text("Countdown  : %.2f [sec]", timeLimit_->countDown_);

	ImGui::End();
	ImGuiManager::GetInstance()->End();
}

// 描画処理
void GameScene::Draw() {
	skydome_->Draw();
	wind_->Draw();

	for (ChainBomb* chainBomb : chainBombs_) {
		chainBomb->Draw();
	}

	player_->Draw();
	enemy_->Draw();

	timeLimit_->Draw();
	fade_.Draw();
}

void GameScene::CheckAllCollisions() {
	// ... (衝突判定のロジックは変更なし) ...
	// プレイヤー
	AABB aabbPlayer = player_->GetAABB();
	AABB aabbEnemy = enemy_->GetAABB();
	AABB aabbWind = wind_->GetAABB();

#pragma region プレイヤーと風の当たり判定
	if (IsCollision(aabbPlayer, aabbWind)) {
		player_->OnCollision(wind_);
	}
#pragma endregion

#pragma region プレイヤーと敵の当たり判定
	if (IsCollision(aabbPlayer, aabbEnemy)) {
		player_->OnCollision(enemy_);
		enemy_->OnCollision(player_);
	}
#pragma endregion

#pragma region プレイヤーと連鎖ボムの当たり判定
	for (ChainBomb* chainBomb : chainBombs_) {
		AABB aabbChainBomb = chainBomb->GetAABB(player_->GetSize());
		if (IsCollision(aabbPlayer, aabbChainBomb)) {
			chainBomb->OnCollision(player_);
		}
	}
#pragma endregion

#pragma region 敵と連鎖ボムの当たり判定
	for (ChainBomb* chainBomb : chainBombs_) {
		if (!chainBomb->IsExplode()) {
			continue;
		}
		AABB aabbChainBomb = chainBomb->GetAABB(player_->GetSize());
		if (IsCollision(aabbEnemy, aabbChainBomb)) {
			chainBomb->OnCollision(enemy_);
			enemy_->OnCollision(chainBomb);
		}
	}
#pragma endregion

#pragma region プレイヤーとビームの当たり判定
	if (enemy_->GetBeam() != nullptr) {
		AABB aabbBeam = enemy_->GetBeam()->GetAABB();
		if (IsCollision(aabbPlayer, aabbBeam)) {
			player_->OnCollision(enemy_->GetBeam());
		}
	}
#pragma endregion

#pragma region プレイヤーと針の当たり判定
	if (!enemy_->GetNeedles().empty()) {
		for (Needle* needle : enemy_->GetNeedles()) {
			OBB obbNeedle = needle->GetOBB();
			if (IsCollisionOBB(aabbPlayer, obbNeedle)) {
				player_->OnCollision(needle);
			}
		}
	}
#pragma endregion

#pragma region プレイヤーとパンチの当たり判定
	if (!enemy_->GetPunches().empty()) {
		for (Punch* punch : enemy_->GetPunches()) {
			AABB aabbPunch = punch->GetAABB();
			if (IsCollision(aabbPlayer, aabbPunch)) {
				player_->OnCollision(punch);
			}
		}
	}
#pragma endregion

#pragma region プレイヤーと雷の当たり判定
	if (!enemy_->GetThunders().empty()) {
		for (Thunder* thunder : enemy_->GetThunders()) {
			if (thunder->IsCollisionDisabled()) {
				continue;
			}
			AABB aabbThunder = thunder->GetAABB();
			if (IsCollision(aabbPlayer, aabbThunder)) {
				player_->OnCollision(thunder);
			}
		}
	}
#pragma endregion
}

bool GameScene::IsCollision(const AABB& aabb1, const AABB& aabb2) {
	if (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x && aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y && aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z) {
		return true;
	}
	return false;
}

bool GameScene::IsCollisionOBB(const AABB& aabb, const OBB& obb) {
	// OBBとAABBの中心点を計算
	Vector3 aabbCenter = { (aabb.min.x + aabb.max.x) / 2.0f, (aabb.min.y + aabb.max.y) / 2.0f, (aabb.min.z + aabb.max.z) / 2.0f };

	// 中心間のベクトル
	Vector3 T;
	T.x = aabbCenter.x - obb.center.x;
	T.y = aabbCenter.y - obb.center.y;
	T.z = aabbCenter.z - obb.center.z;

	// AABBの半分の長さ
	Vector3 aabbHalfSize = { (aabb.max.x - aabb.min.x) / 2.0f, (aabb.max.y - aabb.min.y) / 2.0f, (aabb.max.z - aabb.min.z) / 2.0f };

	// ----------------------------------------------------
	// OBBの3つの軸 (obb.orientations[0..2]) をテスト
	// ----------------------------------------------------
	for (int i = 0; i < 3; ++i) {
		const Vector3& L = obb.orientations[i];

		// ★修正: worldTransform_.Dot(...) -> Dot(...)
		float D = std::abs(Dot(T, L));

		float R_obb;
		if (i == 0) {
			R_obb = obb.size.x / 2.0f;
		}
		else if (i == 1) {
			R_obb = obb.size.y / 2.0f;
		}
		else { // i == 2
			R_obb = obb.size.z / 2.0f;
		}

		// ★修正: worldTransform_.Dot(...) -> Dot(...)
		float R_aabb = std::abs(aabbHalfSize.x * Dot({ 1.0f, 0.0f, 0.0f }, L)) +
			std::abs(aabbHalfSize.y * Dot({ 0.0f, 1.0f, 0.0f }, L)) +
			std::abs(aabbHalfSize.z * Dot({ 0.0f, 0.0f, 1.0f }, L));

		if (D > R_obb + R_aabb) {
			return false;
		}
	}

	// ----------------------------------------------------
	// AABBの3つの軸 (ワールドX, Y, Z) をテスト
	// ----------------------------------------------------
	Vector3 aabbAxes[3] = {
		{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f}
	};

	for (int i = 0; i < 3; ++i) {
		const Vector3& L = aabbAxes[i];

		// ★修正: worldTransform_.Dot(...) -> Dot(...)
		float D = std::abs(Dot(T, L));

		// ★修正: worldTransform_.Dot(...) -> Dot(...)
		float R_obb = std::abs((obb.size.x / 2.0f) * Dot(obb.orientations[0], L)) +
			std::abs((obb.size.y / 2.0f) * Dot(obb.orientations[1], L)) +
			std::abs((obb.size.z / 2.0f) * Dot(obb.orientations[2], L));

		float R_aabb;
		if (i == 0) {
			R_aabb = aabbHalfSize.x;
		}
		else if (i == 1) {
			R_aabb = aabbHalfSize.y;
		}
		else {
			R_aabb = aabbHalfSize.z;
		}

		if (D > R_obb + R_aabb) {
			return false;
		}
	}

	return true;
}

// ... (UpdateFadeIn, UpdateMain, UpdateFadeOut はそのまま) ...
// フェードインの更新
void GameScene::UpdateFadeIn() {
	fade_.Update();

	if (fade_.IsFinished()) {
		phase_ = Phase::kMain;
		fade_.Stop();
	}
}
// メインの更新
void GameScene::UpdateMain() {

	// 天球の更新
	skydome_->Update();

	if (!enemy_->IsDeath()) {

		timeLimit_->Update();
	}

	if (enemy_->IsDead()) {
		isClear_ = true;
		phase_ = Phase::kFadeOut;
		fade_.Start(Fade::Status::kFadeOut, duration_);
	}

	if (timeLimit_->IsTImeUp()) {
		isGameover_ = true;
		phase_ = Phase::kFadeOut;
		fade_.Start(Fade::Status::kFadeOut, duration_);
	}

	if (!timeLimit_->IsStartCountDown() && !timeLimit_->IsTImeUp()) {

		player_->SetIsBlow(false);

		// 風の更新処理
		wind_->Update();

		if (player_->IsRespawning() || player_->IsInvincible()) {
			enemy_->SetThunderEnabled(false);
		}
		else {
			enemy_->SetThunderEnabled(true);
		}

		// 敵の更新処理
		enemy_->Update();

		const float chainRadius = player_->GetSize();

		// 連鎖ボムの更新
		for (ChainBomb* chainBomb : chainBombs_) {
			chainBomb->Update();

			chainBomb->ExplodeAround(chainBombs_, chainRadius);
		}

		// 当たり判定
		CheckAllCollisions();

		// プレイヤーの更新処理
		player_->Update();
	}
}
// フェードアウトの更新
void GameScene::UpdateFadeOut() {
	fade_.Update();

	if (fade_.IsFinished()) {
		isFinished_ = true;
	}
}