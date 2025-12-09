#pragma once
#include "Object3d.h"
#include "Collision.h"
#include "Math/MyMath.h"
#include "Rand.h"

// 攻撃・エフェクトクラスのヘッダー
// (これらはTDEngine用に移植済みである前提です)
#include "Beam.h"
#include "Needle.h"
#include "Thunder.h"
#include "Punch.h"
#include "DeathEx.h"
#include "EnemyDeathParticle.h"

#include <list>
#include <vector>
#include <numbers>
#include "AudioManager.h"

class Player;
class ChainBomb;
class EnemyHPGauge;

// 敵の向き
enum class Direction {
	kLeft,
	kRight,
	kFront,
};

class Enemy {
public:
	// デストラクタ
	~Enemy();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="position">初期座標</param>
	// Model* や Camera* はObject3dが内部で管理するため引数から削除
	void Initialize(const MyMath::Vector3& position);

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

	// コリジョン無効フラグの取得
    bool IsCollisionDisabled() const { return isCollisionDisabled_; }

	// プレイヤーのポインタのセッター
	void SetPlayer(Player* player) { player_ = player; }

	// 攻撃のポインタのゲッター
	Beam* GetBeam() { return beam_; }
	std::list<Needle*> GetNeedles() { return needles_; }
	std::list<Punch*> GetPunches() { return punches_; }
	std::list<Thunder*> GetThunders() { return thunders_; }

	void SetThunderEnabled(bool enabled) { canUseThunder_ = enabled; }

	// 敵の死亡フラグ
	bool IsDeath() const { return behavior_ == Behavior::kDeath; }

	// 振る舞いリクエストを決定
	void SetRequest(bool isUnknown) { isUnknown_ = isUnknown; }

	float GetHP() const { return hp_; }
    float GetHalfHP() const { return halfHp_; }

private:
	// --- 行動制御メソッド ---
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

	// HP処理
	void PlayerHitDamage(const Player& player);
	void BombHitDamage();
	void HitTimer();

private:
	// --- TDEngine用オブジェクト ---
	Object3d* object3d_ = nullptr;

	// モデルの向き
	Direction direction_ = Direction::kLeft;
	float turnFirstRotationY = 0.0f;
	float turnTimer_ = 0.0f;
	static inline const float kTimeTurn = 0.3f;

	// 色制御
	float targetAlpha_ = 0.0f;
	float minAlpha_ = 0.5f;
	float alphaRange_ = 0.5f;
	MyMath::Vector4 color_ = { 1,1,1,1 };

	// 速度
	MyMath::Vector3 velocity_ = {};

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 3.2f;
	static inline const float kHeight = 3.2f;

	// HP
	float hp_ = 100;
	float halfHp_ = 60;

	EnemyHPGauge *hpGauge_ = nullptr;

	// ヒットタイマー
	int hitTimer_ = 100;
	int hitTimerMax_ = 100;
	bool isHit_ = false;

	// 状態フラグ
	bool isChangeStart_ = false;
	bool isChanged_ = false;
	bool isDead_ = false;
	bool isCollisionDisabled_ = false;

	// 振る舞い定義
	enum class Behavior {
		kRoot, kBound, kRound, kBeam, kApproach,
		kNeedle, kThunder, kPunch, kDeath, kChange, kStart, kUnknown
	};
	Behavior behavior_ = Behavior::kStart;
	Behavior behaviorRequest_ = Behavior::kStart;
	bool isUnknown_ = false;
	bool canUseThunder_ = true;

	// 攻撃フェーズ
	enum class AttackPhase { kReservoir, kAttack, kLingering };
	AttackPhase attackPhase_ = AttackPhase::kReservoir;

	// 各種タイマー
	float attackReservoirTimer_ = 40.0f;
	float attackRushTimer_ = 50.0f;
	float attackLingeringTimer_ = 40.0f;
	float attackAfterTimer_ = 10.0f;

	// ランダム
	Rand* rand_ = nullptr;
	int randomValue = 0;

	// 行動履歴
	Behavior preBehavior_ = Behavior::kUnknown;
	Behavior prePreBehavior_ = Behavior::kUnknown;

	// 補間用
	float t = 0.0f;

	// 座標・回転制御
	MyMath::Vector3 initPos_ = { 0.0f, 0.0f, 0.0f };
	MyMath::Vector3 enemyRotate_ = { 0.0f, 0.0f, 0.0f };
	MyMath::Vector3 enemySpeed_ = { 0.0f, 0.0f, 0.0f };
	MyMath::Vector3 enemySpeedDecay_ = { 0.0f, 0.0f, 0.0f };
	static inline const float attackVelocity = 2.0f;

	uint32_t attackParameter_ = 0;
	uint32_t attackCount_ = 0;

	// プレイヤー参照
	Player* player_ = nullptr;

	// --- 攻撃オブジェクト ---
	// ※Modelポインタは各クラス内部でロードさせるため削除

	Beam* beam_ = nullptr;

	std::list<Needle*> needles_;
	static const int kNeedleCount = 4;
	std::vector<MyMath::Vector3> needleRotates_ = {
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, std::numbers::pi_v<float> / 4.2f},
		{0.0f, 0.0f, std::numbers::pi_v<float> / 2.0f},
		{0.0f, 0.0f, std::numbers::pi_v<float> / 1.3f}
	};

	std::list<Thunder*> thunders_;
	static const int kThunderCount = 4;
	std::vector<MyMath::Vector3> thunderPositions_ = {
		{-28.0f, 20.0f, 0.0f}, {-13.0f, 20.0f, 0.0f},
		{3.0f,   20.0f, 0.0f}, {16.0f,  20.0f, 0.0f}
	};

	std::list<Punch*> punches_;
	static const int kPunchCount = 2;
	std::vector<MyMath::Vector3> punchPositions_ = {
		{-10.0f, 0.0f, 0.0f}, {-20.0f, 0.0f, 0.0f}
	};

	std::list<DeathEx*> deathExs_;
	static const int kDeathExCount = 3;
	std::vector<MyMath::Vector3> deathExRotates_ = {
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, std::numbers::pi_v<float> / 4.2f},
		{0.0f, 0.0f, std::numbers::pi_v<float> / 2.2f}
	};

	std::list<EnemyDeathParticle*> deathParticles_;
	static const int kDeathParticleCount = 5;

	// 演出パラメータ
	float changeColorTimer_ = 0.0f;
	float blinkSpeed_ = 0.2f;
	MyMath::Vector3 originalScale_ = { 2.0f, 2.0f, 2.0f };
	MyMath::Vector3 changeScale_ = { 3.0f, 3.0f, 3.0f };

	// SE
	SoundData boundSe_;
	SoundData approachSe_;
	SoundData beamSe_;
	SoundData needleSe_;
	SoundData thunderSe_;
	SoundData deathSe_;
	SoundData changeSe_;
	IXAudio2SourceVoice* pBgmVoice_ = nullptr;
};