#pragma once
#include "AABB.h"
#include "Enemy.h"
#include "Skydome.h"
#include "Fade.h"
#include "TDEngine.h" // TDEngineヘッダー

class Player;
class ChainBomb;
class Wind;
class TimeLimit;

using namespace TDEngine;

class GameScene {
private:
	enum class Phase {
		kFadeIn,  // フェードイン
		kMain,    // メイン
		kFadeOut, // フェードアウト
	};

	Phase phase_ = Phase::kMain;

private:
	// カメラ
	TDEngine::Camera camera_;

	// プレイヤーのポインタ
	Player* player_ = nullptr;
	// プレイヤーのモデル
	TDEngine::Model* modelPlayer_ = nullptr;
	// プレイヤーの位置
	const Vector3 playerPos_ = { -27, 0, 0 };

	// 敵
	Enemy* enemy_ = nullptr;
	// 敵のモデル
	TDEngine::Model* modelEnemy_ = nullptr;
	// 敵の位置
	Vector3 enemyPos_ = { 0, 20, 0 };

	// 連鎖ボム
	std::vector<ChainBomb*> chainBombs_;
	// 連鎖ボムのモデル
	TDEngine::Model* modelChainBomb_ = nullptr;
	// 連鎖ボムの位置
	Vector3 chainBombPos_ = { 0, 0, 0 };

	// 風
	Wind* wind_ = nullptr;
	// 風のモデル
	Model* modelWind_ = nullptr;
	// 風の位置
	Vector3 windPos_ = { 12.2f, -10.0f };

	// 天球
	Skydome* skydome_ = nullptr;

	// 天球のモデル
	Model* modelSkydome_ = nullptr;


	// 時間制限
	TimeLimit* timeLimit_ = nullptr;

	// フェード
	Fade fade_;
	float duration_ = 0.5f;

	// シーンの終了フラグ
	bool isFinished_ = false;
	bool isClear_ = false;
	bool isGameover_ = false;

	// ワールド変換データ
	WorldTransform worldTransform_;

public:
	// デストラクタ
	~GameScene();

	// 初期化処理
	void Initialize();

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

	// 全ての当たり判定を行う
	void CheckAllCollisions();
	// 当たり判定
	bool IsCollision(const AABB& aabb1, const AABB& aabb2);
	// AABBとOBBの当たり判定
	bool IsCollisionOBB(const AABB& aabb, const OBB& obb);

public:
	// シーンの終了フラグのGetter
	bool isFinished() const { return isFinished_; }
	bool IsClear() const { return isClear_; }
	bool IsGameover() const { return isGameover_; }

private:
	// フェードインの更新
	void UpdateFadeIn();
	// メインの更新
	void UpdateMain();
	// フェードアウトの更新
	void UpdateFadeOut();
};