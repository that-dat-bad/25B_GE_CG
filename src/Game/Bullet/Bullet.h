#pragma once
#include "../../engine/base/Math/MyMath.h"
#include "../../engine/Physics/Collider.h"

/// @brief 弾丸エンティティ（オブジェクトプールで管理される）
class Bullet {
public:
	Bullet() = default;
	~Bullet() = default;

	/// @brief 弾丸を発射状態にする
	/// @param position 初期位置
	/// @param direction 発射方向（正規化済み）
	/// @param speed 弾速 (m/s)
	/// @param damage 1発のダメージ
	void Spawn(const MyMath::Vector3& position, const MyMath::Vector3& direction, float speed, float damage);

	/// @brief 弾丸の位置更新
	void Update(float dt);

	/// @brief 弾丸を強制消滅
	void Kill() { isAlive_ = false; }

	// === アクセッサ ===
	bool IsAlive() const { return isAlive_; }
	MyMath::Vector3 GetPosition() const { return position_; }
	float GetDamage() const { return damage_; }

	/// @brief 衝突判定用のコライダーを取得
	SphereCollider GetCollider() const;

private:
	MyMath::Vector3 position_{};
	MyMath::Vector3 velocity_{};
	float damage_ = 0.0f;
	float lifeTime_ = 5.0f;     // 最大生存時間（秒）
	float currentTime_ = 0.0f;
	bool isAlive_ = false;

	static constexpr float kCollisionRadius = 0.5f;
};
