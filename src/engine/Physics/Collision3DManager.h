#pragma once
#include <list>
#include "Collider.h"
#include "ICollisionBody.h"

/// @brief 3D衝突判定マネージャ
/// 登録されたコライダーを毎フレーム総当たり判定し、コールバックを呼ぶ
class Collision3DManager {
public:
	/// @brief コライダーを登録する（毎フレーム呼ぶ）
	/// @param body 判定に参加するオブジェクト
	void Register(ICollisionBody3D* body);

	/// @brief 登録済みの全コライダーの総当たり判定を実行
	/// 衝突があれば双方の OnCollision() を呼ぶ
	void CheckAllCollisions();

	/// @brief 登録リストをクリア（フレーム開始時に呼ぶ）
	void Clear();

	/// @brief 現在の登録数を取得
	size_t GetBodyCount() const { return bodies_.size(); }

	// ============================================================
	// ジオメトリユーティリティ (static)
	// ============================================================

	/// @brief 球と球の衝突判定
	static bool CheckSphereSphere(const SphereCollider& a, const SphereCollider& b);

	/// @brief AABBとAABBの衝突判定
	static bool CheckAABBAABB(const AABBCollider& a, const AABBCollider& b);

	/// @brief 球とAABBの衝突判定
	static bool CheckSphereAABB(const SphereCollider& sphere, const AABBCollider& aabb);

	/// @brief 座標が地面より下かどうか
	/// @param position 判定座標
	/// @param groundY 地面の高さ（デフォルト 0）
	static bool CheckGroundCollision(const MyMath::Vector3& position, float groundY = 0.0f);

private:
	/// @brief 属性/マスクの組み合わせで判定すべきかチェック
	static bool ShouldCheckCollision(const ICollisionBody3D* a, const ICollisionBody3D* b);

	/// @brief 2つのボディの形状に応じた衝突判定
	static bool CheckBodyCollision(ICollisionBody3D* a, ICollisionBody3D* b);

	std::list<ICollisionBody3D*> bodies_;
};
