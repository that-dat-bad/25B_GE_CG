#include "Collision.h"
#include <cmath>
#include <algorithm>

using namespace MyMath;

bool Collision::IsCollision(const AABB& aabb1, const AABB& aabb2) {
	if (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x &&
		aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y &&
		aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z) {
		return true;
	}
	return false;
}

bool Collision::IsCollisionOBB(const AABB& aabb, const OBB& obb) {
	// AABBの中心点
	Vector3 aabbCenter = {
		(aabb.min.x + aabb.max.x) / 2.0f,
		(aabb.min.y + aabb.max.y) / 2.0f,
		(aabb.min.z + aabb.max.z) / 2.0f
	};

	// 中心間のベクトル
	Vector3 T = Substract(aabbCenter, obb.center);

	// AABBの半分の長さ
	Vector3 aabbHalfSize = {
		(aabb.max.x - aabb.min.x) / 2.0f,
		(aabb.max.y - aabb.min.y) / 2.0f,
		(aabb.max.z - aabb.min.z) / 2.0f
	};

	// ----------------------------------------------------
	// OBBの3つの軸 (obb.orientations[0..2]) をテスト
	// ----------------------------------------------------
	for (int i = 0; i < 3; ++i) {
		const Vector3& L = obb.orientations[i];

		// 中心間ベクトルの投影距離
		float D = std::abs(Dot(T, L));

		// OBBの半径
		float R_obb;
		if (i == 0) R_obb = obb.size.x / 2.0f;
		else if (i == 1) R_obb = obb.size.y / 2.0f;
		else R_obb = obb.size.z / 2.0f;

		// AABB の投影半径 R_aabb
		// AABBの軸は (1,0,0), (0,1,0), (0,0,1) なので Dot 計算を簡略化可能だが、汎用的に記述
		float R_aabb = std::abs(aabbHalfSize.x * Dot({ 1.0f, 0.0f, 0.0f }, L)) +
			std::abs(aabbHalfSize.y * Dot({ 0.0f, 1.0f, 0.0f }, L)) +
			std::abs(aabbHalfSize.z * Dot({ 0.0f, 0.0f, 1.0f }, L));

		if (D > R_obb + R_aabb) {
			return false;
		}
	}

	// ----------------------------------------------------
	// AABBの3つの軸 (ワールドX, Y, Z) をテスト
	// ----------------------------------------------------
	Vector3 aabbAxes[3] = {
		{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f}
	};

	for (int i = 0; i < 3; ++i) {
		const Vector3& L = aabbAxes[i];

		float D = std::abs(Dot(T, L));

		// OBB の投影半径 R_obb
		float R_obb = std::abs((obb.size.x / 2.0f) * Dot(obb.orientations[0], L)) +
			std::abs((obb.size.y / 2.0f) * Dot(obb.orientations[1], L)) +
			std::abs((obb.size.z / 2.0f) * Dot(obb.orientations[2], L));

		// AABB の投影半径
		float R_aabb;
		if (i == 0) R_aabb = aabbHalfSize.x;
		else if (i == 1) R_aabb = aabbHalfSize.y;
		else R_aabb = aabbHalfSize.z;

		if (D > R_obb + R_aabb) {
			return false;
		}
	}

	return true;
}