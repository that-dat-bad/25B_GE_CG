#pragma once
#include "Math/MyMath.h"
#include "Object3d.h"
#include "Collision.h"
#include <list>
#include <vector>
#include <memory>

// 前方宣言
class Player;
class GameScene;
class ChainBomb;
class Beam;
class Needle;
class Punch;
class Thunder;
class EnemyDeathParticle;
class DeathEx;
class Rand;

// 敵の向き
enum class Direction {
	kLeft,
	kRight,
	kFront,
};

class Enemy {
public:
	~Enemy();

	// 初期化
	void Initialize(const MyMath::Vector3& position);

	// 更新
	void Update();

	// 描画
	void Draw();

	// 衝突応答
	void OnCollision(const Player* player);
	void OnCollision(const ChainBomb* chainBomb);

	// ゲッター・セッター
	MyMath::Vector3 GetWorldPosition() const;
	AABB GetAABB();
	bool IsDead() const { return isDead_; }
	bool IsCollisionDisabled() const { return isCollisionDisabled_; }
	bool IsDeath() const { return behavior_ == Behavior::kDeath; }

	void SetPlayer(Player* player) { player_ = player; }
	void SetThunderEnabled(bool enabled) { canUseThunder_ = enabled; }
	void SetRequest(bool isUnknown) { isUnknown_ = isUnknown; }

	// 攻撃オブジェクト取得 (Collision判定用)
	Beam* GetBeam() const { return beam_; }
	const std::list<Needle*>& GetNeedles() const { return needles_; }
	const std::list<Punch*>& GetPunches() const { return punches_; }
	const std::list<Thunder*>& GetThunders() const { return thunders_; }

private:
	// TDEngine用 Object3d
	Object3d* object3d_ = nullptr;

	// パラメータ
	MyMath::Vector3 velocity_ = {};
	static inline const float kWidth = 3.2f;
	static inline const float kHeight = 3.2f;

	float hp_ = 100.0f; // 初期値適当（元コードではInitializeで設定されていなかったので）
	float halfHp_ = 50.0f;

	// ヒット演出
	int hitTimer_ = 0;
	static const int hitTimerMax_ = 10; // 元コードのロジックに合わせて調整
	bool isHit_ = false;

	// 状態フラグ
	bool isChangeStart_ = false;
	bool isChanged_ = false; // 初期状態はfalse, 変化後にtrue
	bool isDead_ = false;
	bool isCollisionDisabled_ = false;
	bool canUseThunder_ = true;

	// 旋回制御
	Direction direction_ = Direction::kLeft;
	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;
	static inline const float kTimeTurn = 0.3f;

	// 行動パターン
	enum class Behavior {
		kRoot, kBound, kRound, kBeam, kApproach,
		kNeedle, kThunder, kPunch, kDeath, kChange, kStart, kUnknown
	};
	Behavior behavior_ = Behavior::kStart;
	Behavior behaviorRequest_ = Behavior::kStart;
	bool isUnknown_ = false;

	// 攻撃フェーズ
	enum class AttackPhase { kReservoir, kAttack, kLingering };
	AttackPhase attackPhase_ = AttackPhase::kReservoir;

	// タイマー系
	float attackReservoirTimer_ = 0.0f;
	float attackRushTimer_ = 0.0f;
	float attackLingeringTimer_ = 0.0f;
	float attackAfterTimer_ = 0.0f;
	uint32_t attackParameter_ = 0;
	uint32_t attackCount_ = 0;

	// 制御用変数
	Rand* rand_ = nullptr;
	int randomValue_ = 0;
	Behavior preBehavior_ = Behavior::kUnknown;
	Behavior prePreBehavior_ = Behavior::kUnknown;
	float t_ = 0.0f; // 補間用

	MyMath::Vector3 initPos_ = {};
	MyMath::Vector3 enemyRotate_ = {};
	MyMath::Vector3 enemySpeed_ = {};
	MyMath::Vector3 enemySpeedDecay_ = {};

	// プレイヤー参照
	Player* player_ = nullptr;

	// 攻撃オブジェクト
	Beam* beam_ = nullptr;
	std::list<Needle*> needles_;
	std::list<Thunder*> thunders_;
	std::list<Punch*> punches_;

	// エフェクト
	std::list<DeathEx*> deathExs_;
	std::list<EnemyDeathParticle*> deathParticles_;

	// 変身演出用
	float changeColorTimer_ = 0.0f;
	float blinkSpeed_ = 0.2f;
	MyMath::Vector3 originalScale_ = { 2.0f, 2.0f, 2.0f };
	MyMath::Vector3 changeScale_ = { 3.0f, 3.0f, 3.0f };
	MyMath::Vector4 color_ = { 1,1,1,1 };
	float targetAlpha_ = 0.0f;
	float minAlpha_ = 0.5f;
	float alphaRange_ = 0.5f;

	// 攻撃設定データ（角度など）
	std::vector<MyMath::Vector3> needleRotates_;
	std::vector<MyMath::Vector3> thunderPositions_;
	std::vector<MyMath::Vector3> punchPositions_;
	std::vector<MyMath::Vector3> deathExRotates_;

private:
	// 各ステートの初期化・更新メソッド
	void BehaviorRootInitialize();   void BehaviorRootUpdate();
	void BehaviorBoundInitialize();  void BehaviorBoundUpdate();
	void BehaviorRoundInitialize();  void BehaviorRoundUpdate();
	void BehaviorBeamInitialize();   void BehaviorBeamUpdate();
	void BehaviorApproachInitialize(); void BehaviorApproachUpdate();
	void BehaviorNeedleInitialize(); void BehaviorNeedleUpdate();
	void BehaviorThunderInitialize(); void BehaviorThunderUpdate();
	void BehaviorPunchInitialize();  void BehaviorPunchUpdate();
	void BehaviorDeathInitialize();  void BehaviorDeathUpdate();
	void BehaviorChangeInitialize(); void BehaviorChangeUpdate();
	void BehaviorStartInitialize();  void BehaviorStartUpdate();

	void PlayerHitDamage(const Player& player);
	void BombHitDamage();
	void HitTimer();
};