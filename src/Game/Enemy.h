#pragma once
#include "AABB.h"
#include "Beam.h"
#include "DeathEx.h"
#include "EnemyDeathParticle.h"
#include "Needle.h"
#include "Punch.h"
#include "Rand.h"
#include "Thunder.h"
#include <TDEngine.h>
#include <numbers>
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
	/// <param name="model">モデル</param>
	/// <param name="position">初期座標</param>

	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>

	void Update();

	/// <summary>
	/// 描画
	/// </summary>

	void Draw();

	// ワールド座標の取得
	TDEngine::Vector3 GetWorldPosition();
	// AABBを取得
	AABB GetAABB();
	// 衝突応答
	void OnCollision(const Player* player_);
	void OnCollision(const ChainBomb* chainBomb_);
	// デスフラグの取得
	bool IsDead() const { return isDead_; }
	// 通常行動の更新
	void BehaviorRootUpdate();
	// 通常行動の初期化
	void BehaviorRootInitialize();
	// 跳ね回りの初期化
	void BehaviorBoundInitialize();
	// 跳ね回りの更新
	void BehaviorBoundUpdate();
	// 往復の初期化
	void BehaviorRoundInitialize();
	// 往復の更新
	void BehaviorRoundUpdate();
	// ビーム攻撃の初期化
	void BehaviorBeamInitialize();
	// ビーム攻撃の更新
	void BehaviorBeamUpdate();
	// 接近の初期化
	void BehaviorApproachInitialize();
	// 接近の更新
	void BehaviorApproachUpdate();
	// 針攻撃の初期化
	void BehaviorNeedleInitialize();
	// 針攻撃の更新
	void BehaviorNeedleUpdate();
	// 雷攻撃の初期化
	void BehaviorThunderInitialize();
	// 雷攻撃の更新
	void BehaviorThunderUpdate();
	// 連続パンチの初期化
	void BehaviorPunchInitialize();
	// 連続パンチの更新
	void BehaviorPunchUpdate();
	// デス演出の更新
	void BehaviorDeathUpdate();
	// デス演出の初期化
	void BehaviorDeathInitialize();
	// 形態変化演出の更新
	void BehaviorChangeUpdate();
	// 形態変化演出の初期化
	void BehaviorChangeInitialize();
	// 開始演出の初期化
	void BehaviorStartInitialize();
	// 開始演出の更新
	void BehaviorStartUpdate();
	// コリジョン無効フラグの取得
	bool IsCollisionDisabled() const { return isCollisionDisabled_; }
	// 敵の死亡フラグ
	bool IsDeath() const { return behavior_ == Behavior::kDeath; }
	// プレイヤーのポインタのセッター
	void SetPlayer(Player* player) { player_ = player; }
	// 攻撃のポインタのゲッター
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
	TDEngine::WorldTransform worldTransform_;
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
	TDEngine::ObjectColor objectColor_;
	TDEngine::Vector4 color_;

	// カメラ
	TDEngine::Camera* camera_ = nullptr;
	// テクスチャハンドル
	uint32_t textureHandle_ = 0;

	// 速度
	TDEngine::Vector3 velocity_;

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

	// 振る舞い
	Behavior behavior_ = Behavior::kStart;

	// 次の振る舞いリクエスト
	Behavior behaviorRequest_ = Behavior::kStart;

	// 振る舞いリクエスト決定フラグ
	bool isUnknown_ = false;

	bool canUseThunder_ = true;

	// 攻撃フェーズ
	enum class AttackPhase {
		kReservoir, // 溜め
		kAttack,    // 攻撃
		kLingering  // 余韻
	};

	// 現在の攻撃フェーズ
	AttackPhase attackPhase_;

	// 溜め動作のタイマー
	float attackReservoirTimer_ = 40.0f;

	// 突進動作のタイマー
	float attackRushTimer_ = 50.0f;

	// 余韻動作のタイマー
	float attackLingeringTimer_ = 40.0f;

	// 攻撃の後隙タイマー
	float attackAfterTimer_ = 10.0f;

	// ランダム用ポインタ
	Rand* rand_ = nullptr;
	int randomValue = 0;

	// 以前の行動フェーズ
	Behavior preBehavior_ = Behavior::kUnknown;
	// 二つ前の行動フェーズ
	Behavior prePreBehavior_ = Behavior::kUnknown;

	// 線形補間用の変数
	float t = 0.0f;

	// 初期位置
	TDEngine::Vector3 initPos_ = {0.0f, 0.0f, 0.0f};

	// 敵の角度
	TDEngine::Vector3 enemyRotate_ = {0.0f, 0.0f, 0.0f};

	// 移動速度
	TDEngine::Vector3 enemySpeed_ = {0.0f, 0.0f, 0.0f};

	// 移動速度の減衰率
	TDEngine::Vector3 enemySpeedDecay_ = {0.0f, 0.0f, 0.0f};

	// 攻撃速度
	static inline const float attackVelocity = 2.0f;

	// 攻撃ギミックの経過カウンター
	uint32_t attackParameter_ = 0;

	// 攻撃回数のカウント
	uint32_t attackCount_ = 0;

	// コリジョン無効フラグ
	bool isCollisionDisabled_ = false;
	// ゲームシーンのポインタ
	GameScene* gameScene_;
	// プレイヤーのポインタ
	Player* player_;

	// ビーム
	Beam* beam_ = nullptr;
	// ビームのモデル
	TDEngine::Model* modelBeam_ = nullptr;

	// 針
	std::list<Needle*> needles_;
	// 針の数
	static const int kNeedleCount = 4;
	// 針の角度リスト
	std::vector<TDEngine::Vector3> needleRotates_ = {
	    {0.0f, 0.0f, 0.0f	                        },
        {0.0f, 0.0f, std::numbers::pi_v<float> / 4.2f},
        {0.0f, 0.0f, std::numbers::pi_v<float> / 2.0f},
        {0.0f, 0.0f, std::numbers::pi_v<float> / 1.3f}
    };
	// 針のモデル
	TDEngine::Model* modelNeedle_ = nullptr;

	// 雷
	std::list<Thunder*> thunders_;
	// 雷の数
	static const int kThunderCount = 4;
	// 雷の位置リスト
	std::vector<TDEngine::Vector3> thunderPositions_ = {
	    {-28.0f, 20.0f, 0.0f},
        {-13.0f, 20.0f, 0.0f},
        {3.0f,   20.0f, 0.0f},
        {16.0f,  20.0f, 0.0f}
    };
	// 雷のモデル
	TDEngine::Model* modelThunder_ = nullptr;

	// 連続パンチ
	std::list<Punch*> punches_;
	// 連続パンチの数
	static const int kPunchCount = 2;
	// 連続パンチの位置リスト
	std::vector<TDEngine::Vector3> punchPositions_ = {
	    {-10.0f, 0.0f, 0.0f},
        {-20.0f, 0.0f, 0.0f}
    };
	// 連続パンチのモデル
	TDEngine::Model* modelPunch_ = nullptr;

	// デス爆発パーティクル
	std::list<DeathEx*> deathExs_;
	// デス爆発パーティクルの数
	static const int kDeathExCount = 3;
	// デス爆発パーティクルの角度リスト
	std::vector<TDEngine::Vector3> deathExRotates_ = {
	    {0.0f, 0.0f, 0.0f	                        },
        {0.0f, 0.0f, std::numbers::pi_v<float> / 4.2f},
        {0.0f, 0.0f, std::numbers::pi_v<float> / 2.2f}
    };
	// デス爆発パーティクルのモデル
	TDEngine::Model* modelDeathEx_ = nullptr;

	// デスパーティクル
	std::list<EnemyDeathParticle*> deathParticles_;
	// デスパーティクルの数
	static const int kDeathParticleCount = 5;
	// デスパーティクルのモデル
	TDEngine::Model* modelDeathParticle_ = nullptr;

	// 形態変化演出の色変化タイマー
	float changeColorTimer_ = 0.0f;
	// 点滅の速さ
	float blinkSpeed_ = 0.2f;
	// 形態変化前のサイズ
	TDEngine::Vector3 originalScale_ = {2.0f, 2.0f, 2.0f};
	// 形態変化後のサイズ
	TDEngine::Vector3 changeScale_ = {3.0f, 3.0f, 3.0f};
};