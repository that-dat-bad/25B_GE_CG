#include "GameScene.h"
#include "TDEngine.h" // Input, CameraManager, Managersへのアクセス
#include "CameraManager.h"
#include "Collision.h" // 移植したCollisionクラス

// 各クラスのヘッダー (TDEngine対応版)
#include "Player.h"
#include "Enemy.h"
#include "ChainBomb.h" // ※Object3d化が必要
#include "Wind.h"      // ※Object3d化が必要
#include "Skydome.h"   // ※Object3d化が必要
#include "TimeLimit.h" // ※Sprite化が必要
#include "Fade.h"      // ※Sprite化が必要
#include "Beam.h"
#include "Needle.h"
#include "Punch.h"
#include "Thunder.h"
// ImGui
#include "ImGuiManager.h"

using namespace MyMath;

// デストラクタ
GameScene::~GameScene() {
	delete player_;
	delete enemy_;
	for (ChainBomb* chainBomb : chainBombs_) {
		delete chainBomb;
	}
	chainBombs_.clear();
	delete wind_;
	delete skydome_;
	delete timeLimit_;
	delete fade_;
}

// 初期化処理
void GameScene::Initialize() {
	// カメラ初期化 (CameraManagerを利用)
	// "GameCamera" という名前で作成し、アクティブにする
	CameraManager::GetInstance()->CreateCamera("GameCamera");
	CameraManager::GetInstance()->SetActiveCamera("GameCamera");

	// カメラの位置設定 (TDEngineのCameraクラスの仕様に合わせる)
	Camera* camera = CameraManager::GetInstance()->GetActiveCamera();
	camera->SetTranslate({ 0.0f, 0.0f, -50.0f }); // 適切な距離に設定
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });

	// 天球の初期化
	skydome_ = new Skydome();
	skydome_->Initialize();

	// プレイヤーの初期化
	player_ = new Player();
	player_->Initialize(playerPos_);

	// 敵の初期化
	enemy_ = new Enemy();
	enemy_->Initialize(enemyPos_);
	enemy_->SetPlayer(player_); // 敵にプレイヤー情報を渡す

	// 連鎖ボムの初期化
	for (int32_t i = 0; i < 5; ++i) {
		ChainBomb* chainBomb = new ChainBomb();
		Vector3 chainBombPos = { -5.0f + i * 5.0f, 0.0f + i * 1.3f, 0.0f };
		chainBomb->Initialize(chainBombPos);
		chainBombs_.push_back(chainBomb);
	}

	// 風の初期化
	wind_ = new Wind();
	wind_->Initialize(windPos_);

	// UI: 時間制限
	timeLimit_ = new TimeLimit();
	timeLimit_->Initialize();

	// UI: フェード
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::kFadeIn, duration_);

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

	// TDEngineではカメラの行列更新はEngine側(Updateループ内)で呼ばれるため
	// ここで明示的に呼ぶ必要はないが、独自の追従ロジックがあれば記述する

	// ImGuiデバッグ表示 (TDEngine::GetImGuiManager()のようなアクセスが必要だが、
	// ImGuiの関数はどこでも呼べるため直接記述)
#ifdef _DEBUG
	ImGui::Begin("GameScene");
	if (timeLimit_) {
		ImGui::Text("Timer: %.2f", timeLimit_->timer_);
	}
	ImGui::End();
#endif
}

// 描画処理
void GameScene::Draw() {
	// 3Dオブジェクト描画準備 (パイプライン設定など)
	TDEngine::GetObject3dCommon()->SetupCommonState();

	// 天球
	if (skydome_) skydome_->Draw();

	// 風
	if (wind_) wind_->Draw();

	// 連鎖ボム
	for (ChainBomb* chainBomb : chainBombs_) {
		chainBomb->Draw();
	}

	// プレイヤー
	if (player_) player_->Draw();

	// 敵
	if (enemy_) enemy_->Draw();

	// --- 2Dスプライト描画 ---
	// スプライト共通設定の適用
	TDEngine::GetSpriteCommon()->SetupCommonState();

	// UI類
	if (timeLimit_) timeLimit_->Draw();
	if (fade_) fade_->Draw();
}

// 全ての当たり判定を行う
void GameScene::CheckAllCollisions() {
	if (!player_ || !enemy_ || !wind_) return;

	// プレイヤーのAABB
	AABB aabbPlayer = player_->GetAABB();
	// 敵のAABB
	AABB aabbEnemy = enemy_->GetAABB();
	// 風のAABB
	AABB aabbWind = wind_->GetAABB();

	// 1. プレイヤーと風
	if (Collision::IsCollision(aabbPlayer, aabbWind)) {
		player_->OnCollision(wind_);
	}

	// 2. プレイヤーと敵
	if (Collision::IsCollision(aabbPlayer, aabbEnemy)) {
		player_->OnCollision(enemy_);
		enemy_->OnCollision(player_);
	}

	// 3. プレイヤーと連鎖ボム
	for (ChainBomb* chainBomb : chainBombs_) {
		// ボム側の判定サイズロジックに合わせる
		AABB aabbChainBomb = chainBomb->GetAABB();
		if (Collision::IsCollision(aabbPlayer, aabbChainBomb)) {
			chainBomb->OnCollision(player_);
		}
	}

	// 4. 敵と連鎖ボム
	for (ChainBomb* chainBomb : chainBombs_) {
		if (!chainBomb->IsExplode()) continue;

		AABB aabbChainBomb = chainBomb->GetAABB();
		if (Collision::IsCollision(aabbEnemy, aabbChainBomb)) {
			chainBomb->OnCollision(enemy_);
			enemy_->OnCollision(chainBomb);
		}
	}

	// 5. プレイヤーとビーム
	if (enemy_->GetBeam() != nullptr) {
		AABB aabbBeam = enemy_->GetBeam()->GetAABB();
		if (Collision::IsCollision(aabbPlayer, aabbBeam)) {
			player_->OnCollision(enemy_->GetBeam());
		}
	}

	// 6. プレイヤーと針 (OBB判定)
	for (auto* needle : enemy_->GetNeedles()) {
		if (!needle) continue;
		OBB obbNeedle = needle->GetOBB();
		if (Collision::IsCollisionOBB(aabbPlayer, obbNeedle)) {
			player_->OnCollision(needle);
		}
	}

	// 7. プレイヤーとパンチ
	for (auto* punch : enemy_->GetPunches()) {
		if (!punch) continue;
		AABB aabbPunch = punch->GetAABB();
		if (Collision::IsCollision(aabbPlayer, aabbPunch)) {
			player_->OnCollision(punch);
		}
	}

	// 8. プレイヤーと雷
	for (auto* thunder : enemy_->GetThunders()) {
		if (!thunder) continue;
		if (thunder->IsCollisionDisabled()) continue;

		AABB aabbThunder = thunder->GetAABB();
		if (Collision::IsCollision(aabbPlayer, aabbThunder)) {
			player_->OnCollision(thunder);
		}
	}
}

// フェードインの更新
void GameScene::UpdateFadeIn() {
	if (fade_) fade_->Update();

	if (skydome_) skydome_->Update();

	if (fade_ && fade_->IsFinished()) {
		phase_ = Phase::kMain;
		fade_->Stop();
	}
}

// メインの更新
void GameScene::UpdateMain() {
	if (skydome_) skydome_->Update();

	if (enemy_ && !enemy_->IsDeath()) {
		if (timeLimit_) timeLimit_->Update();
	}

	// クリア判定
	if (enemy_ && enemy_->IsDead()) {
		isClear_ = true;
		phase_ = Phase::kFadeOut;
		if (fade_) fade_->Start(Fade::Status::kFadeOut, duration_);
	}

	// ゲームオーバー判定
	if (timeLimit_ && timeLimit_->IsTImeUp()) {
		isGameover_ = true;
		phase_ = Phase::kFadeOut;
		if (fade_) fade_->Start(Fade::Status::kFadeOut, duration_);
	}

	// ゲームプレイ中
	if (timeLimit_ && !timeLimit_->IsStartCountDown() && !timeLimit_->IsTImeUp()) {

		player_->SetIsBlow(false);

		if (wind_) wind_->Update();

		if (player_->IsRespawning() || player_->IsInvincible()) {
			enemy_->SetThunderEnabled(false);
		}
		else {
			enemy_->SetThunderEnabled(true);
		}

		if (enemy_) enemy_->Update();

		const float chainRadius = player_->GetSize();
		for (ChainBomb* chainBomb : chainBombs_) {
			chainBomb->Update();
			chainBomb->ExplodeAround(chainBombs_, chainRadius);
		}

		CheckAllCollisions();

		if (player_) player_->Update();
	}
}

// フェードアウトの更新
void GameScene::UpdateFadeOut() {
	if (fade_) fade_->Update();
	if (skydome_) skydome_->Update();

	if (fade_ && fade_->IsFinished()) {
		isFinished_ = true;
	}
}