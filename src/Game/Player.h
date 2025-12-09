#pragma once
#include "Math/MyMath.h"
#include "Object3d.h"
#include "Collision.h"
#include <list>
#include <memory>
#include "AudioManager.h"

// 前方宣言
class Enemy;
class Wind;
class Beam;
class Needle;
class Punch;
class Thunder;
class EnemyDeathParticle; 
class FireworkParticle;
class Rand;

class Player {
public:
	// 初期化 (CameraはObject3dCommonで管理されるため引数から削除、もしくはセット用)
	void Initialize(const MyMath::Vector3& position);

	// 更新
	void Update();

	// 描画
	void Draw();

	// デストラクタ
	~Player();

	// 衝突応答
	void OnCollision(const Enemy* enemy);
	void OnCollision(const Wind* wind);
	void OnCollision(const Beam* beam);
	void OnCollision(const Needle* needle);
	void OnCollision(const Punch* punch);
	void OnCollision(const Thunder* thunder);

	// ゲッター
	MyMath::Vector3 GetWorldPosition() const;
	AABB GetAABB();
	float GetSize() const { return size_; }
	bool IsExplode() const { return isExplode_; }
	float GetExplosivePower() const { return explosivePower_; }
	MyMath::Vector3 GetVelocity() const { return velocity_; }
	void SetPosition(const MyMath::Vector3& position);
	bool IsBlow() const { return isBlow_; }
	void SetIsBlow(bool isBlow) { isBlow_ = isBlow; }
	MyMath::Vector3 GetScale() const;

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
	Object3d* object3d_ = nullptr;

	// 画面の範囲
	static inline const float kScreenLeft = -36.0f;
	static inline const float kScreenRight = 36.0f;
	static inline const float kScreenBottom = -18.0f;
	static inline const float kScreenTop = 18.0f;

	// ベースサイズ
	float baseSize_ = 2.0f;
	// モデルサイズ
	float size_ = baseSize_;

	// 旋回関連
	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;
	static inline const float kTimeTurn = 0.3f;

	// 生存フラグ
	bool isAlive_ = true;
	// 爆発フラグ
	bool isExplode_ = false;

	static inline const float kDeltaTime = 1.0f / 60.0f;
	static inline const float kRespawnTimeSelf = 1.5f;
	static inline const float kRespawnTimeKilled = 2.5f;
	float respawnTimer_ = 0.0f;

	// リスポーン地点
	MyMath::Vector3 startPos_ = { 0.0f, 0.0f, 0.0f };
	MyMath::Vector3 endPos_ = { 0.0f, 0.0f, 0.0f };

	// 無敵関連
	float invincibleTimer_ = 0.0f;
	static inline const float kInvincibleTime = 0.5f;
	uint32_t frameCount_ = 0;
	static inline const uint32_t kFrameCount = 30;

	// 爆発威力
	float explosivePower_ = baseSize_;

	// 物理パラメータ
	static inline const float kMaxSpeed = 0.3f;
	MyMath::Vector3 velocity_ = {};
	MyMath::Vector3 acceleration_ = { 0, 0, 0 };
	static inline const float kAcceleration = 0.01f;
	static inline const float kAccelerationInvincible = 0.003f;
	static inline const float kAttenuation = 0.05f;

	// スケール制限
	static inline const float kMaxScale = 4.0f;
	static inline const float kMinScale = 1.0f;

	bool isBlow_ = false;

	// エフェクト関連
	EnemyDeathParticle* deathParticle_ = nullptr;
	std::list<FireworkParticle*> fireworkParticles_;
	static const int kFireParticleCount = 3;
	Rand* rand_ = nullptr;

	// SE
	SoundData upSe_;
	SoundData downSe_;
	SoundData clashSe_;
	IXAudio2SourceVoice* pBgmVoice_ = nullptr;

private:
	void UpdateMove();
	void UpdateAlive();
	void UpdateDead();
	void UpdateRespawn();
	void StartRespawn();
	void UpdateInvincible();
	void ApplyScaleFromHeight();
};