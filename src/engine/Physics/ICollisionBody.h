#pragma once
#include <cstdint>
#include "Collider.h"

// ============================================================
// 3D 衝突ボディ インターフェース
// ============================================================

/// @brief 3D衝突判定に参加するオブジェクトが実装するインターフェース
/// 
/// 使い方:
/// 1. ゲームオブジェクトがこのクラスを継承する
/// 2. 属性 (Attribute) で「自分が何か」を、マスク (Mask) で「何と当たるか」を指定
/// 3. マネージャに Register() して UpdateAll() で自動判定
class ICollisionBody3D {
public:
	virtual ~ICollisionBody3D() = default;

	/// @brief 自分のコライダー形状の種類を返す
	virtual ColliderType3D GetColliderType3D() const { return ColliderType3D::Sphere; }

	/// @brief 球コライダーを返す（形状が Sphere の場合）
	virtual SphereCollider GetSphereCollider() const { return {}; }

	/// @brief AABBコライダーを返す（形状が AABB の場合）
	virtual AABBCollider GetAABBCollider() const { return {}; }

	/// @brief 自分の衝突属性ビットを返す（CollisionAttribute の値）
	virtual uint32_t GetCollisionAttribute() const = 0;

	/// @brief 衝突対象のマスクビットを返す（CollisionMask の値）
	virtual uint32_t GetCollisionMask() const = 0;

	/// @brief 衝突時に呼ばれるコールバック
	/// @param other 衝突した相手
	virtual void OnCollision([[maybe_unused]] ICollisionBody3D* other) {}

	/// @brief この判定ボディが有効かどうか（falseなら判定スキップ）
	virtual bool IsCollisionActive() const { return true; }
};

// ============================================================
// 2D 衝突ボディ インターフェース
// ============================================================

/// @brief 2D衝突判定に参加するオブジェクトが実装するインターフェース
/// UI・マップ判定用
class ICollisionBody2D {
public:
	virtual ~ICollisionBody2D() = default;

	/// @brief 自分のコライダー形状の種類を返す
	virtual ColliderType2D GetColliderType2D() const { return ColliderType2D::Rect; }

	/// @brief 円コライダーを返す（形状が Circle の場合）
	virtual CircleCollider GetCircleCollider() const { return {}; }

	/// @brief 矩形コライダーを返す（形状が Rect の場合）
	virtual RectCollider GetRectCollider() const { return {}; }

	/// @brief 自分の衝突属性ビットを返す
	virtual uint32_t GetCollisionAttribute() const = 0;

	/// @brief 衝突対象のマスクビットを返す
	virtual uint32_t GetCollisionMask() const = 0;

	/// @brief 衝突時に呼ばれるコールバック
	/// @param other 衝突した相手
	virtual void OnCollision([[maybe_unused]] ICollisionBody2D* other) {}

	/// @brief この判定ボディが有効かどうか
	virtual bool IsCollisionActive() const { return true; }
};
