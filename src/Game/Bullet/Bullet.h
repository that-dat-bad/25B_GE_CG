#pragma once
#include "../../engine/base/Math/MyMath.h"
#include "../../engine/Physics/Collider.h"
#include "../../engine/Physics/ICollisionBody.h"
#include "../../engine/Physics/CollisionConfig.h"

/// <summary>
/// 弾丸エンティティ（オブジェクトプールで管理される）
/// </summary>
class Bullet : public ICollisionBody3D {
public:
	Bullet() = default;
	~Bullet() = default;

	/// <summary>
	/// 弾丸を発射状態にする
	/// </summary>
	/// <param name="position">初期位置</param>
	/// <param name="velocity">弾丸の初速度ベクトル (m/s)</param>
	/// <param name="damage">1発のダメージ</param>
	/// <param name="isEnemyBullet">敵の弾丸かどうか（デフォルト false）</param>
	void Spawn(const MyMath::Vector3& position, const MyMath::Vector3& velocity, float damage, bool isEnemyBullet = false);

	/// <summary>
	/// 弾丸の位置更新
	/// </summary>
	void Update(float dt);

	/// <summary>
	/// 弾丸を強制消滅
	/// </summary>
	void Kill() { isAlive_ = false; }

	// === アクセッサ ===
	bool IsAlive() const { return isAlive_; }
	MyMath::Vector3 GetPosition() const { return position_; }
	MyMath::Vector3 GetVelocity() const { return velocity_; }
	float GetDamage() const { return damage_; }
	bool IsEnemyBullet() const { return isEnemyBullet_; }
	
	static constexpr int kMaxHistory = 4;
	const MyMath::Vector3* GetHistory() const { return history_; }
	int GetHistoryCount() const { return historyCount_; }

	/// <summary>
	/// 衝突判定用のコライダーを取得
	/// </summary>
	SphereCollider GetCollider() const;

	// === ICollisionBody3D 実装 ===
	SphereCollider GetSphereCollider() const override { return GetCollider(); }
	uint32_t GetCollisionAttribute() const override {
		return isEnemyBullet_ ? CollisionAttribute::kEnemyBullet : CollisionAttribute::kPlayerBullet;
	}
	uint32_t GetCollisionMask() const override {
		return isEnemyBullet_ ? CollisionMask::kEnemyBullet : CollisionMask::kPlayerBullet;
	}
	void OnCollision(ICollisionBody3D* other) override;
	bool IsCollisionActive() const override { return isAlive_; }

private:
	MyMath::Vector3 position_{};
	MyMath::Vector3 velocity_{};
	float damage_ = 0.0f;
	float lifeTime_ = 5.0f;     // 最大生存時間（秒）
	float currentTime_ = 0.0f;
	bool isAlive_ = false;
	bool isEnemyBullet_ = false;

	MyMath::Vector3 history_[kMaxHistory]{};
	int historyCount_ = 0;

	static constexpr float kCollisionRadius = 0.5f;
};
