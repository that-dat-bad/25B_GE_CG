#pragma once
#include "../base/Math/MyMath.h"

// ============================================================
// 3D コライダー
// ============================================================

/// @brief コライダーの種類（3D）
enum class ColliderType3D {
	Sphere,
	AABB,
};

/// @brief 球コライダー
struct SphereCollider {
	MyMath::Vector3 center;
	float radius = 1.0f;
};

/// @brief AABBコライダー
struct AABBCollider {
	MyMath::Vector3 min;
	MyMath::Vector3 max;
};

// ============================================================
// 2D コライダー
// ============================================================

/// @brief コライダーの種類（2D）
enum class ColliderType2D {
	Circle,
	Rect,
};

/// @brief 円コライダー（2D）
struct CircleCollider {
	Vector2 center;
	float radius = 1.0f;
};

/// @brief 矩形コライダー（2D / AABB）
struct RectCollider {
	Vector2 min;
	Vector2 max;
};
