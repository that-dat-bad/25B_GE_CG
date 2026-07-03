#pragma once
#include "../base/Math/MyMath.h"

// ============================================================
// 3D コライダー
// ============================================================

/// <summary>
/// コライダーの種類（3D）
/// </summary>
enum class ColliderType3D {
	Sphere,
	AABB,
};

/// <summary>
/// 球コライダー
/// </summary>
struct SphereCollider {
	MyMath::Vector3 center;
	float radius = 1.0f;
};

/// <summary>
/// AABBコライダー
/// </summary>
struct AABBCollider {
	MyMath::Vector3 min;
	MyMath::Vector3 max;
};

// ============================================================
// 2D コライダー
// ============================================================

/// <summary>
/// コライダーの種類（2D）
/// </summary>
enum class ColliderType2D {
	Circle,
	Rect,
};

/// <summary>
/// 円コライダー（2D）
/// </summary>
struct CircleCollider {
	Vector2 center;
	float radius = 1.0f;
};

/// <summary>
/// 矩形コライダー（2D / AABB）
/// </summary>
struct RectCollider {
	Vector2 min;
	Vector2 max;
};
