#pragma once
#include "Collider.h"
#include <vector>

class Bullet;
class Enemy;

/// @brief 衝突判定マネージャー
class CollisionManager {
public:
	/// @brief 球と球の衝突判定
	/// @return 衝突していれば true
	static bool CheckSphereSphere(const SphereCollider& a, const SphereCollider& b);

	/// @brief 自機と地面の衝突判定
	/// @param position 判定する座標
	/// @param groundY 地面の高さ（デフォルト 0）
	/// @return 地面に衝突していれば true
	static bool CheckGroundCollision(const MyMath::Vector3& position, float groundY = 0.0f);

	/// @brief 弾丸リストと敵リストの一括衝突判定
	/// ヒットした弾丸は消滅し、敵にダメージを与える
	static void CheckBulletEnemyCollisions(
		std::vector<Bullet>& bullets,
		const std::vector<Enemy*>& enemies,
		float bulletDamage
	);
};
