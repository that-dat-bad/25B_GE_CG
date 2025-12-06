#define NOMINMAX
#include "GameScene.h"
#include "../External/TDEngine/include/2d/ImGuiManager.h"
#include "ChainBomb.h"
#include "Player.h"
#include "TimeLimit.h"
#include "Wind.h"

#include <algorithm>
#include <limits>

using namespace TDEngine;

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
	// プレイヤーのポインタ
	delete player_;
	// プレイヤーのモデル
	delete modelPlayer_;

	// 敵のポインタ
	delete enemy_;
	// 敵のモデル
	delete modelEnemy_;

	// 連鎖ボムのポインタ
	for (ChainBomb* chainBomb : chainBombs_) {
		delete chainBomb;
	}
	chainBombs_.clear();
	// 連鎖ボムのモデル
	delete modelChainBomb_;

	// 風のポインタ
	delete wind_;

	// 風のモデル
	delete modelWind_;

	// デバッグカメラ
	// delete debugCamera_;
}

// 初期化処理
void GameScene::Initialize() {

	// カメラの初期化
	camera_.Initialize();

	//// デバッグカメラ生成
	// debugCamera_ = new DebugCamera(1280, 720);

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

	// 連鎖ボムの3Dモデルの生成
	modelChainBomb_ = Model::CreateFromOBJ("bomb", true);
	for (int32_t i = 0; i < 5; ++i) {

		// 連鎖ボムの生成
		ChainBomb* chainBomb = new ChainBomb();

		chainBombPos_ = {-5.0f + i * 5.0f, 0.0f + i * 1.3f, 0};

		// 連鎖ボムの初期化
		chainBomb->Initialize(modelChainBomb_, &camera_, chainBombPos_);
		chainBombs_.push_back(chainBomb);
	}

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	// 風の生成
	wind_ = new Wind();
	// 風の3Dモデルの生成
	modelWind_ = Model::CreateFromOBJ("wind", true);
	// 風の初期化
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

	// ビュープロジェクション行列の更新と転送
	camera_.TransferMatrix();

	ImGui::Begin("GameScene");

	/*ImGui::SeparatorText("Player");
	ImGui::Text("Player isBlow_: %s", player_->IsBlow() ? "true" : "false");
	ImGui::Text("Player velocity_");
	ImGui::Text("x: %f, y: %f", player_->GetVelocity().x, player_->GetVelocity().y);
	ImGui::Text("Player Size_: %f", player_->GetSize());


	ImGui::End();*/

	ImGui::Text("Phase      : %s", ToString(timeLimit_->phase_));
	ImGui::Text("Timer      : %.2f [sec]", timeLimit_->timer_);
	ImGui::Text("Countdown  : %.2f [sec]", timeLimit_->countDown_);

	ImGui::End();
}

// 描画処理
void GameScene::Draw() {
	Model::PreDraw();

	// 天球の描画
	skydome_->Draw();

	wind_->Draw();

	// 連鎖ボムの描画
	for (ChainBomb* chainBomb : chainBombs_) {
		chainBomb->Draw();
	}

	// プレイヤーの描画処理
	player_->Draw();

	// 敵の描画処理
	enemy_->Draw();

	Model::PostDraw();

	timeLimit_->Draw();

	fade_.Draw();
}

void GameScene::CheckAllCollisions() {
	// プレイヤー
	AABB aabbPlayer;
	// プレイヤーの座標
	aabbPlayer = player_->GetAABB();

	// 敵
	AABB aabbEnemy;
	// 敵の座標
	aabbEnemy = enemy_->GetAABB();

	// 風
	AABB aabbWind;
	// 風の座標
	aabbWind = wind_->GetAABB();

#pragma region プレイヤーと風の当たり判定
	// AABB同士の交差判定
	if (IsCollision(aabbPlayer, aabbWind)) {
		// プレイヤーの衝突関数
		player_->OnCollision(wind_);
	}
#pragma endregion

	// 攻撃

#pragma region プレイヤーと敵の当たり判定
	// AABB同士の交差判定
	if (IsCollision(aabbPlayer, aabbEnemy)) {
		// プレイヤーの衝突関数
		player_->OnCollision(enemy_);
		// 敵の衝突関数
		enemy_->OnCollision(player_);
	}
#pragma endregion

#pragma region プレイヤーと連鎖ボムの当たり判定
	// プレイヤーと連鎖ボムの当たり判定
	for (ChainBomb* chainBomb : chainBombs_) {

		AABB aabbChainBomb = chainBomb->GetAABB(player_->GetSize());

		if (IsCollision(aabbPlayer, aabbChainBomb)) {
			chainBomb->OnCollision(player_);
		}
	}
#pragma endregion

#pragma region 敵と連鎖ボムの当たり判定
	// 敵と連鎖ボムの当たり判定
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
		// AABB同士の交差判定
		AABB aabbBeam = enemy_->GetBeam()->GetAABB();
		if (IsCollision(aabbPlayer, aabbBeam)) {
			// プレイヤーの衝突関数
			player_->OnCollision(enemy_->GetBeam());
		}
	}

#pragma endregion

#pragma region プレイヤーと針の当たり判定

	if (!enemy_->GetNeedles().empty()) {
		// AABB同士の交差判定
		for (Needle* needle : enemy_->GetNeedles()) {

			OBB obbNeedle = needle->GetOBB();
			if (IsCollisionOBB(aabbPlayer, obbNeedle)) {
				// プレイヤーの衝突関数
				player_->OnCollision(needle);
			}
		}
	}

#pragma endregion

#pragma region プレイヤーとパンチの当たり判定

	if (!enemy_->GetPunches().empty()) {
		// AABB同士の交差判定
		for (Punch* punch : enemy_->GetPunches()) {
			AABB aabbPunch = punch->GetAABB();
			if (IsCollision(aabbPlayer, aabbPunch)) {
				// プレイヤーの衝突関数
				player_->OnCollision(punch);
			}
		}
	}

#pragma endregion

#pragma region プレイヤーと雷の当たり判定

	if (!enemy_->GetThunders().empty()) {
		// AABB同士の交差判定
		for (Thunder* thunder : enemy_->GetThunders()) {

			// コリジョン無効の雷はスキップ
			if (thunder->IsCollisionDisabled()) {
				continue;
			}

			AABB aabbThunder = thunder->GetAABB();
			if (IsCollision(aabbPlayer, aabbThunder)) {
				// プレイヤーの衝突関数
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
	TDEngine::Vector3 aabbCenter = {(aabb.min.x + aabb.max.x) / 2.0f, (aabb.min.y + aabb.max.y) / 2.0f, (aabb.min.z + aabb.max.z) / 2.0f};

	// 中心間のベクトル
	TDEngine::Vector3 T;
	T.x = aabbCenter.x - obb.center.x;
	T.y = aabbCenter.y - obb.center.y;
	T.z = aabbCenter.z - obb.center.z;

	// AABBの半分の長さ
	TDEngine::Vector3 aabbHalfSize = {(aabb.max.x - aabb.min.x) / 2.0f, (aabb.max.y - aabb.min.y) / 2.0f, (aabb.max.z - aabb.min.z) / 2.0f};

	// ----------------------------------------------------
	// OBBの3つの軸 (obb.orientations[0..2]) をテスト
	// ----------------------------------------------------
	for (int i = 0; i < 3; ++i) {
		// 分離軸 L は obb.orientations[i]
		const TDEngine::Vector3& L = obb.orientations[i];

		// 中心間ベクトルの投影距離
		float D = std::abs(worldTransform_.Dot(T, L));

		float R_obb;
		if (i == 0) {
			R_obb = obb.size.x / 2.0f;
		} else if (i == 1) {
			R_obb = obb.size.y / 2.0f;
		} else { // i == 2
			R_obb = obb.size.z / 2.0f;
		}

		// AABB の投影半径 R_aabb
		float R_aabb = std::abs(aabbHalfSize.x * worldTransform_.Dot({1.0f, 0.0f, 0.0f}, L)) + std::abs(aabbHalfSize.y * worldTransform_.Dot({0.0f, 1.0f, 0.0f}, L)) +
		               std::abs(aabbHalfSize.z * worldTransform_.Dot({0.0f, 0.0f, 1.0f}, L));

		// 分離テスト
		if (D > R_obb + R_aabb) {
			return false;
		}
	}

	// ----------------------------------------------------
	// AABBの3つの軸 (ワールドX, Y, Z) をテスト
	// ----------------------------------------------------
	TDEngine::Vector3 aabbAxes[3] = {
	    {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    };

	for (int i = 0; i < 3; ++i) {
		// 分離軸 L は aabbAxes[i] (ワールド軸)
		const TDEngine::Vector3& L = aabbAxes[i];

		// 中心間ベクトルの投影距離
		float D = std::abs(worldTransform_.Dot(T, L));

		// OBB の投影半径 R_obb
		float R_obb = std::abs((obb.size.x / 2.0f) * worldTransform_.Dot(obb.orientations[0], L)) + std::abs((obb.size.y / 2.0f) * worldTransform_.Dot(obb.orientations[1], L)) +
		              std::abs((obb.size.z / 2.0f) * worldTransform_.Dot(obb.orientations[2], L));

		// AABB の投影半径 R_aabb
		float R_aabb;
		if (i == 0) {
			R_aabb = aabbHalfSize.x;
		} else if (i == 1) {
			R_aabb = aabbHalfSize.y;
		} else {
			R_aabb = aabbHalfSize.z;
		}

		// 分離テスト
		if (D > R_obb + R_aabb) {
			return false;
		}
	}

	// ----------------------------------------------------
	// すべての分離軸で重なっていた場合、衝突している
	// ----------------------------------------------------
	return true;
}

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
		} else {
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

		//// デバッグカメラの更新
		// debugCamera_->Update();

		// camera_.matView = debugCamera_->GetCamera().matView;
		// camera_.matProjection = debugCamera_->GetCamera().matProjection;
	}
}
// フェードアウトの更新
void GameScene::UpdateFadeOut() {
	fade_.Update();

	if (fade_.IsFinished()) {
		isFinished_ = true;
	}
}
