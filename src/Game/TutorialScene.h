#pragma once
#include "TDEngine.h" // TDEngine統合ヘッダー
#include "AABB.h"
#include "Fade.h"
#include "Player.h"
#include "Enemy.h"
#include "Skydome.h"

class TutorialScene {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	// 全ての当たり判定を行う
	void CheckAllCollisions();
	// 当たり判定
	bool IsCollision(const AABB& aabb1, const AABB& aabb2);

public:
	// シーン終了フラグのGetter
	bool IsFinished() const { return isFinished_; }

private:
	enum class Phase {
		kFadeIn,  // フェードイン
		kMain,    // メイン
		kFadeOut, // フェードアウト
	};

	Phase phase_ = Phase::kMain;

private:
	// クリアフラグ
	bool isCleared_ = false;

	// フェード
	Fade fade_;
	float duration_ = 0.5f;

	// シーン終了フラグ
	bool isFinished_ = false;

	// カメラ (TDEngine::Camera)
	Camera camera_;

	// プレイヤーのポインタ
	Player* player_ = nullptr;
	// プレイヤーのモデル
	Model* modelPlayer_ = nullptr;
	// プレイヤーの位置
	const Vector3 playerPos_ = { -27, 0, 0 };

	// 敵
	Enemy* enemy_ = nullptr;
	// 敵のモデル
	Model* modelEnemy_ = nullptr;
	// 敵の位置
	Vector3 enemyPos_ = { 0, 20, 0 };

	// シーン切り替えタイマー
	int changeTimer_ = 60;

	// 天球
	Skydome* skydome_ = nullptr;

	// 天球のモデル
	Model* modelSkydome_ = nullptr;


private:
	//フェードインの更新
	void UpdateFadeIn();
	// メインの更新
	void UpdateMain();
	// フェードアウトの更新
	void UpdateFadeOut();
};