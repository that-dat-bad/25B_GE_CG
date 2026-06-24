#include "CollisionManager.h"
#include "../../Game/Bullet/Bullet.h"
#include "../../Game/Enemy/Enemy.h"
#include <cmath>

using namespace MyMath;

bool CollisionManager::CheckSphereSphere(const SphereCollider& a, const SphereCollider& b) {
	Vector3 diff = Substract(a.center, b.center);
	float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
	float radiusSum = a.radius + b.radius;
	return distSq <= radiusSum * radiusSum;
}

bool CollisionManager::CheckGroundCollision(const MyMath::Vector3& position, float groundY) {
	return position.y <= groundY;
}

void CollisionManager::CheckBulletEnemyCollisions(
	std::vector<Bullet>& bullets,
	const std::vector<Enemy*>& enemies,
	float bulletDamage,
	std::vector<MyMath::Vector3>& outHitPositions
) {
	for (auto& bullet : bullets) {
		if (!bullet.IsAlive()) continue;

		SphereCollider bulletCollider = bullet.GetCollider();

		for (auto* enemy : enemies) {
			if (!enemy->IsAlive()) continue;

			SphereCollider enemyCollider;
			enemyCollider.center = enemy->GetPosition();
			enemyCollider.radius = enemy->GetCollisionRadius();

			if (CheckSphereSphere(bulletCollider, enemyCollider)) {
				// ヒット：弾丸を消滅させ、敵にダメージを与える
				bullet.Kill();
				enemy->TakeDamage(bulletDamage);
				outHitPositions.push_back(bullet.GetPosition());
				break; // 1発の弾が複数の敵にヒットしない
			}
		}
	}
}
