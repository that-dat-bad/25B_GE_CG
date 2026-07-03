#include "Collision3DManager.h"
#include <cmath>
#include <algorithm>

// ============================================================
// 登録・クリア
// ============================================================

void Collision3DManager::Register(ICollisionBody3D* body) {
	if (body) {
		bodies_.push_back(body);
	}
}

void Collision3DManager::Clear() {
	bodies_.clear();
}

// ============================================================
// 総当たり判定
// ============================================================

void Collision3DManager::CheckAllCollisions() {
	// 全ペアを総当たりで判定
	for (auto itA = bodies_.begin(); itA != bodies_.end(); ++itA) {
		auto itB = itA;
		++itB;
		for (; itB != bodies_.end(); ++itB) {
			ICollisionBody3D* bodyA = *itA;
			ICollisionBody3D* bodyB = *itB;

			// 非アクティブならスキップ
			if (!bodyA->IsCollisionActive() || !bodyB->IsCollisionActive()) {
				continue;
			}

			// 属性/マスクフィルタリング
			if (!ShouldCheckCollision(bodyA, bodyB)) {
				continue;
			}

			// 形状に応じた衝突判定
			if (CheckBodyCollision(bodyA, bodyB)) {
				// 双方のコールバックを呼ぶ
				bodyA->OnCollision(bodyB);
				bodyB->OnCollision(bodyA);
			}
		}
	}
}

// ============================================================
// 属性/マスクフィルタリング
// ============================================================

bool Collision3DManager::ShouldCheckCollision(const ICollisionBody3D* a, const ICollisionBody3D* b) {
	// 双方が相手の属性を衝突対象として認めている場合のみ判定する
	return (a->GetCollisionAttribute() & b->GetCollisionMask()) != 0 &&
	       (b->GetCollisionAttribute() & a->GetCollisionMask()) != 0;
}

// ============================================================
// 形状判定のディスパッチ
// ============================================================

bool Collision3DManager::CheckBodyCollision(ICollisionBody3D* a, ICollisionBody3D* b) {
	ColliderType3D typeA = a->GetColliderType3D();
	ColliderType3D typeB = b->GetColliderType3D();

	if (typeA == ColliderType3D::Sphere && typeB == ColliderType3D::Sphere) {
		return CheckSphereSphere(a->GetSphereCollider(), b->GetSphereCollider());
	}
	if (typeA == ColliderType3D::AABB && typeB == ColliderType3D::AABB) {
		return CheckAABBAABB(a->GetAABBCollider(), b->GetAABBCollider());
	}
	// Sphere vs AABB（順序を正規化）
	if (typeA == ColliderType3D::Sphere && typeB == ColliderType3D::AABB) {
		return CheckSphereAABB(a->GetSphereCollider(), b->GetAABBCollider());
	}
	if (typeA == ColliderType3D::AABB && typeB == ColliderType3D::Sphere) {
		return CheckSphereAABB(b->GetSphereCollider(), a->GetAABBCollider());
	}

	return false;
}

// ============================================================
// ジオメトリユーティリティ
// ============================================================

bool Collision3DManager::CheckSphereSphere(const SphereCollider& a, const SphereCollider& b) {
	MyMath::Vector3 diff = MyMath::Subtract(a.center, b.center);
	float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
	float radiusSum = a.radius + b.radius;
	return distSq <= radiusSum * radiusSum;
}

bool Collision3DManager::CheckAABBAABB(const AABBCollider& a, const AABBCollider& b) {
	// 各軸で重なりがなければ衝突しない
	if (a.max.x < b.min.x || a.min.x > b.max.x) { return false; }
	if (a.max.y < b.min.y || a.min.y > b.max.y) { return false; }
	if (a.max.z < b.min.z || a.min.z > b.max.z) { return false; }
	return true;
}

bool Collision3DManager::CheckSphereAABB(const SphereCollider& sphere, const AABBCollider& aabb) {
	// 球の中心からAABBへの最近接点を求める
	float closestX = std::clamp(sphere.center.x, aabb.min.x, aabb.max.x);
	float closestY = std::clamp(sphere.center.y, aabb.min.y, aabb.max.y);
	float closestZ = std::clamp(sphere.center.z, aabb.min.z, aabb.max.z);

	float dx = sphere.center.x - closestX;
	float dy = sphere.center.y - closestY;
	float dz = sphere.center.z - closestZ;

	float distSq = dx * dx + dy * dy + dz * dz;
	return distSq <= sphere.radius * sphere.radius;
}

bool Collision3DManager::CheckGroundCollision(const MyMath::Vector3& position, float groundY) {
	return position.y <= groundY;
}
