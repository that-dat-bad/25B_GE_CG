#pragma once
#include "Math/MyMath.h"

// 構造体定義
struct AABB {
	MyMath::Vector3 min;
	MyMath::Vector3 max;
};

struct OBB {
	MyMath::Vector3 center;
	MyMath::Vector3 orientations[3];
	MyMath::Vector3 size;
};

class Collision {
public:
	// AABB同士の衝突判定
	static bool IsCollision(const AABB& aabb1, const AABB& aabb2);

	// AABBとOBBの衝突判定
	static bool IsCollisionOBB(const AABB& aabb, const OBB& obb);
};