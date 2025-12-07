#pragma once
#include "AABB.h"
#include "Beam.h"
#include "DeathEx.h"
#include "EnemyDeathParticle.h"
#include "Needle.h"
#include "Punch.h"
#include "Rand.h"
#include "Thunder.h"
#include "TDEngine.h" // TDEngine統合ヘッダー
#include <numbers>
#include <list>
#include <vector>

class Player;
class ChainBomb;
class GameScene;

// 敵の向き
enum class Direction {
	kLeft,
	kRight,
	kFront,
};

class Enemy {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const MyMath::Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	// ワールド座標の取得
	MyMath::Vector3 GetWorldPosition();
	// AABBを取得
	AABB GetAABB();
	// 衝突応答
	void OnCollision(const Player* player_);
	void OnCollision(const ChainBomb* chainBomb_);
	// デスフラグの取得
	bool IsDead() const { return isDead_; }

	// 各行動の関数
	void BehaviorRootUpdate();
	void BehaviorRootInitialize();
	void BehaviorBoundInitialize();
	void BehaviorBoundUpdate();
	void BehaviorRoundInitialize();
	void BehaviorRoundUpdate();
	void BehaviorBeamInitialize();
	void BehaviorBeamUpdate();
	void BehaviorApproachInitialize();
	void BehaviorApproachUpdate();
	void BehaviorNeedleInitialize();
	void BehaviorNeedleUpdate();
	void BehaviorThunderInitialize();
	void BehaviorThunderUpdate();
	void BehaviorPunchInitialize();
	void BehaviorPunchUpdate();
	void BehaviorDeathUpdate();
	void BehaviorDeathInitialize();
	void BehaviorChangeUpdate();
	void BehaviorChangeInitialize();
	void BehaviorStartInitialize();
	void BehaviorStartUpdate();

	bool IsCollisionDisabled() const { return isCollisionDisabled_; }
	bool IsDeath() const { return behavior_ == Behavior::kDeath; }
	void SetPlayer(Player* player) { player_ = player; }

	Beam* GetBeam() { return beam_; }
	std::list<Needle*> GetNeedles() { return needles_; }
	std::list<Punch*> GetPunches() { return punches_; }
	std::list<Thunder*> GetThunders() { return thunders_; }

	void SetThunderEnabled(bool enabled) { canUseThunder_ = enabled; }

	// HPを減らす
	void PlayerHitDamage(const Player& player);
	void BombHitDamage();
	// ヒットタイマー
	void HitTimer();
	// デストラクタ
	~Enemy();

	// 振る舞いリクエストを決定
	void SetRequest(bool isUnknown) { isUnknown_ = isUnknown; }

private:
	// ワールド変換データ
	WorldTransform worldTransform_;
	// モデル
	TDEngine::Model* model_ = nullptr;
	// モデルの向き
	Direction direction_ = Direction::kLeft;
	// 旋回開始時の角度
	float turnFirstRotationY = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;
	// 旋回時間(秒)
	static inline const float kTimeTurn = 0.3f;
	// モデルのアルファ値
	float targetAlpha_ = 0.0f;
	float minAlpha_ = 0.5f;
	float alphaRange_ = 0.5f;

	// ObjectColorはTDEngineにないので削除し、Vector4で管理
	MyMath::Vector4 color_;

	// カメラ
	TDEngine::Camera* camera_ = nullptr;

	// 速度
	MyMath::Vector3 velocity_;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 3.2f;
	static inline const float kHeight = 3.2f;

	// HP
	float hp_ = 0;
	float halfHp_ = 50;

	// ヒットタイマー
	int hitTimer_ = 100;
	int hitTimerMax_ = 100;

	// ヒットフラグ
	bool isHit_ = false;

	// 形態変化フラグ
	bool isChangeStart_ = false;
	bool isChanged_ = true;

	// デスフラグ
	bool isDead_ = false;

	// 振る舞い
	enum class Behavior {
		kRoot,     // 通常行動
		kBound,    // 跳ね回る
		kRound,    // 往復
		kBeam,     // ビーム攻撃
		kApproach, // 接近
		kNeedle,   // 針攻撃
		kThunder,  // 雷攻撃
		kPunch,    // 連続パンチ
		kDeath,    // デス演出
		kChange,   // 形態変化
		kStart,    // 開始演出
		kUnknown   // 未定義
	};

	Behavior behavior_ = Behavior::kStart;
	Behavior behaviorRequest_ = Behavior::kStart;
	bool isUnknown_ = false;
	bool canUseThunder_ = true;

	// 攻撃フェーズ
	enum class AttackPhase {
		kReservoir, // 溜め
		kAttack,    // 攻撃
		kLingering  // 余韻
	};

	AttackPhase attackPhase_;

	float attackReservoirTimer_ = 40.0f;
	float attackRushTimer_ = 50.0f;
	float attackLingeringTimer_ = 40.0f;
	float attackAfterTimer_ = 10.0f;

	Rand* rand_ = nullptr;
	int randomValue = 0;

	Behavior preBehavior_ = Behavior::kUnknown;
	Behavior prePreBehavior_ = Behavior::kUnknown;

	float t = 0.0f;

	MyMath::Vector3 initPos_ = { 0.0f, 0.0f, 0.0f };
	MyMath::Vector3 enemyRotate_ = { 0.0f, 0.0f, 0.0f };
	MyMath::Vector3 enemySpeed_ = { 0.0f, 0.0f, 0.0f };
	MyMath::Vector3 enemySpeedDecay_ = { 0.0f, 0.0f, 0.0f };

	static inline const float attackVelocity = 2.0f;
	uint32_t attackParameter_ = 0;
	uint32_t attackCount_ = 0;

	bool isCollisionDisabled_ = false;
	GameScene* gameScene_;
	Player* player_;

	// 各種モデルとオブジェクト
	Beam* beam_ = nullptr;
	TDEngine::Model* modelBeam_ = nullptr;

	std::list<Needle*> needles_;
	static const int kNeedleCount = 4;
	std::vector<MyMath::Vector3> needleRotates_ = {
		{0.0f, 0.0f, 0.0f	                        },
		{0.0f, 0.0f, std::numbers::pi_v<float> / 4.2f},
		{0.0f, 0.0f, std::numbers::pi_v<float> / 2.0f},
		{0.0f, 0.0f, std::numbers::pi_v<float> / 1.3f}
	};
	TDEngine::Model* modelNeedle_ = nullptr;

	std::list<Thunder*> thunders_;
	static const int kThunderCount = 4;
	std::vector<MyMath::Vector3> thunderPositions_ = {
		{-28.0f, 20.0f, 0.0f},
		{-13.0f, 20.0f, 0.0f},
		{3.0f,   20.0f, 0.0f},
		{16.0f,  20.0f, 0.0f}
	};
	TDEngine::Model* modelThunder_ = nullptr;

	std::list<Punch*> punches_;
	static const int kPunchCount = 2;
	std::vector<MyMath::Vector3> punchPositions_ = {
		{-10.0f, 0.0f, 0.0f},
		{-20.0f, 0.0f, 0.0f}
	};
	TDEngine::Model* modelPunch_ = nullptr;

	std::list<DeathEx*> deathExs_;
	static const int kDeathExCount = 3;
	std::vector<MyMath::Vector3> deathExRotates_ = {
		{0.0f, 0.0f, 0.0f	                        },
		{0.0f, 0.0f, std::numbers::pi_v<float> / 4.2f},
		{0.0f, 0.0f, std::numbers::pi_v<float> / 2.2f}
	};
	TDEngine::Model* modelDeathEx_ = nullptr;

	std::list<EnemyDeathParticle*> deathParticles_;
	static const int kDeathParticleCount = 5;
	TDEngine::Model* modelDeathParticle_ = nullptr;

	float changeColorTimer_ = 0.0f;
	float blinkSpeed_ = 0.2f;
	MyMath::Vector3 originalScale_ = { 2.0f, 2.0f, 2.0f };
	MyMath::Vector3 changeScale_ = { 3.0f, 3.0f, 3.0f };
};