
#pragma once
#include "AABB.h"
#include "EnemyDeathParticle.h"
#include "FireworkParticle.h"
#include "Rand.h"
#include <TDEngine.h>

class Enemy;
class Wind;
class Beam;
class Needle;
class Punch;
class Thunder;

class Player {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	// 衝突応答
	void OnCollision(const Enemy* enemy);
	void OnCollision(const Wind* wind);
	void OnCollision(const Beam* beam);
	void OnCollision(const Needle* needle);
	void OnCollision(const Punch* punch);
	void OnCollision(const Thunder* thunder);

	const TDEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }
	TDEngine::Vector3 GetWorldPosition();
	/// <summary>
	/// 当たり判定
	/// </summary>
	/// <returns></returns>
	AABB GetAABB();

	/// <summary>
	/// サイズ
	/// </summary>
	/// <returns></returns>
	float GetSize() { return size_; }

	/// <summary>
	/// 爆発フラグ
	/// </summary>
	/// <returns></returns>
	bool IsExplode() const { return isExplode_; }
	/// <summary>
	/// 爆発威力
	/// </summary>
	/// <returns></returns>
	float GetExplosivePower() { return explosivePower_; }

	TDEngine::Vector3 GetVelocity() const { return velocity_; }

	void SetPosition(const TDEngine::Vector3& position) { worldTransform_.translation_ = position; }

	bool IsBlow() const { return isBlow_; }
	void SetIsBlow(bool isBlow) { isBlow_ = isBlow; }

	// プレイヤーの大きさを取得
	TDEngine::Vector3 GetScale() const { return worldTransform_.scale_; }

	bool IsRespawning() const { return state_ == State::kRespawn; }
	bool IsInvincible() const { return state_ == State::kInvincible; }
	bool IsDead() const { return state_ == State::kDead; }

private:
	enum class State {
		kAlive,   // 生存中
		kDead,    // 死亡中
		kRespawn, // リスポーン中
		kInvincible,//無敵時間
	};

	State state_ = State::kAlive;

	enum class LRDirection {
		kRight, // 右向き
		kLeft,  // 左向き
	};

	LRDirection lrDirection_ = LRDirection::kRight;

private:
	// カメラ
	TDEngine::Camera* camera_ = nullptr;

	// 画面の範囲
	static inline const float kScreenLeft = -36.0f;
	static inline const float kScreenRight = 36.0f;
	static inline const float kScreenBottom = -18.0f;
	static inline const float kScreenTop = 18.0f;

	// モデル
	TDEngine::Model* model_ = nullptr;
	// ベースサイズ
	float baseSize_ = 2.0f;
	// モデルサイズ
	float size_ = baseSize_;

	// ワールド変換データ
	TDEngine::WorldTransform worldTransform_;

	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;
	// 旋回時間（秒）
	static inline const float kTimeTurn = 0.3f;

	// 生存フラグ
	bool isAlive_ = true;
	// 爆発フラグ
	bool isExplode_ = false;

	static inline const float kDeltaTime = 1.0f / 60.0f;
	// 自爆時のリスポーンまでの時間
	static inline const float kRespawnTimeSelf = 1.5f;
	// 倒されたときのリスポーンまでの時間
	static inline const float kRespawnTimeKilled = 2.5f;
	// リスポーンまでのカウントダウン
	float respawnTimer_ = 0.0f;

	// リスポーンスタート地点
	TDEngine::Vector3 startPos_ = {0.0f, 0.0f, 0.0f};
	// リスポーン終了地点
	TDEngine::Vector3 endPos_ = {0.0f, 0.0f, 0.0f};

	//無敵時間
	float invincibleTimer_ = 0.0f;
	//無敵継続時間
	static inline const float kInvincibleTime = 0.5f;
	//点滅
	uint32_t frameCount_ = 0;
	static inline const uint32_t kFrameCount = 30;

	// 爆発威力
	float explosivePower_ = baseSize_;

	// 最大速度
	static inline const float kMaxSpeed = 0.3f;
	// 速度
	TDEngine::Vector3 velocity_ = {};
	// 加速度
	TDEngine::Vector3 acceleration_ = {0, 0, 0};
	// 加速度定数
	static inline const float kAcceleration = 0.01f;
	//無敵時は減速
	static inline const float kAccelerationInvincible = 0.003f;

	// 速度減衰率
	static inline const float kAttenuation = 0.05f;

	// 伸縮の最大値
	static inline const float kMaxScale = 4.0f;
	// 伸縮の最小値
	static inline const float kMinScale = 1.0f;

	bool isBlow_ = false;

	// デスパーティクル
	EnemyDeathParticle* deathParticle_ = nullptr;

	// デスパーティクルのモデル
	TDEngine::Model* modelParticle_ = nullptr;

	// 爆発パーティクル
	std::list<FireworkParticle*> fireworkParticles_;
	// 爆発パーティクルの数
	static const int kFireParticleCount = 3;
	// 爆発パーティクルのモデル
	TDEngine::Model* modelFireParticle_ = nullptr;
	// ランダム用ポインタ
	Rand* rand_ = nullptr;
	int randomValue = 0;

private:
	/// <summary>
	/// 移動入力処理
	/// </summary>
	void UpdateMove();

	// 生存中の更新
	void UpdateAlive();
	// 死亡時の更新
	void UpdateDead();
	// リスポーン時の更新
	void UpdateRespawn();

	// リスポーンスタート
	void StartRespawn();

	//無敵中の更新
	void UpdateInvincible();

	void ApplyScaleFromHeight();

	
	/// <summary>
	/// イージング
	/// </summary>
	/// <param name="t"></param>
	/// <returns></returns>
	float EaseInOutSine(float t);
};
