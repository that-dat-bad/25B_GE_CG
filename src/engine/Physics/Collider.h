#pragma once
#include "../base/Math/MyMath.h"

/// @brief コライダーの種類
enum class ColliderType {
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
