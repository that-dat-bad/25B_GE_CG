#pragma once
#include <list>
#include "Collider.h"
#include "ICollisionBody.h"

/// @brief 2D衝突判定マネージャ
/// UI・マップ要素の衝突判定を管理する
class Collision2DManager {
public:
	/// @brief コライダーを登録する（毎フレーム呼ぶ）
	/// @param body 判定に参加するオブジェクト
	void Register(ICollisionBody2D* body);

	/// @brief 登録済みの全コライダーの総当たり判定を実行
	void CheckAllCollisions();

	/// @brief 登録リストをクリア（フレーム開始時に呼ぶ）
	void Clear();

	/// @brief 現在の登録数を取得
	size_t GetBodyCount() const { return bodies_.size(); }

	// ============================================================
	// ジオメトリユーティリティ (static)
	// ============================================================

	/// @brief 円と円の衝突判定
	static bool CheckCircleCircle(const CircleCollider& a, const CircleCollider& b);

	/// @brief 矩形と矩形の衝突判定（AABB 2D）
	static bool CheckRectRect(const RectCollider& a, const RectCollider& b);

	/// @brief 円と矩形の衝突判定
	static bool CheckCircleRect(const CircleCollider& circle, const RectCollider& rect);

	/// @brief 点が矩形内にあるか（UI クリック判定用）
	static bool CheckPointRect(const Vector2& point, const RectCollider& rect);

	/// @brief 点が円内にあるか
	static bool CheckPointCircle(const Vector2& point, const CircleCollider& circle);

private:
	/// @brief 属性/マスクの組み合わせで判定すべきかチェック
	static bool ShouldCheckCollision(const ICollisionBody2D* a, const ICollisionBody2D* b);

	/// @brief 2つのボディの形状に応じた衝突判定
	static bool CheckBodyCollision(ICollisionBody2D* a, ICollisionBody2D* b);

	std::list<ICollisionBody2D*> bodies_;
};
