#include "Needle.h"
#include <cassert>

using namespace TDEngine;
using namespace MyMath;

// 簡易的なイージング関数
Vector3 EaseInVec3(float t, const Vector3& start, const Vector3& end) {
	return {
		start.x + (end.x - start.x) * t * t,
		start.y + (end.y - start.y) * t * t,
		start.z + (end.z - start.z) * t * t
	};
}
Vector3 NormalizeVec3(const Vector3& v) {
	float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	if (len == 0) return v;
	return { v.x / len, v.y / len, v.z / len };
}


void Needle::Initialize(Model* model, Camera* camera, const Vector3& position, const Vector3 rotate) {
	assert(model);
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation = position;
	worldTransform_.rotation = rotate;

	// 初期位置調整
	worldTransform_.translation.x = 2.0f;
	worldTransform_.translation.y = 2.0f;
}

void Needle::Update() {
	t += 0.01f;
	// サイズをイージング (EaseIn関数を使用)
	worldTransform_.scale = EaseInVec3(t, worldTransform_.scale, upScale_);
	worldTransform_.UpdateMatrix();
}

void Needle::Draw() {
	model_->Draw(worldTransform_, *camera_);
}

Vector3 Needle::GetWorldPosition() {
	return worldTransform_.translation;
}

OBB Needle::GetOBB() {
	OBB obb;
	obb.center = GetWorldPosition();

	const Vector3& currentScale = worldTransform_.scale;
	obb.size.x = width * currentScale.x;
	obb.size.y = height * currentScale.y;
	obb.size.z = width * currentScale.z;

	const Matrix4x4& matWorld = worldTransform_.matWorld;

	// 軸の取得と正規化
	Vector3 xAxis = { matWorld.m[0][0], matWorld.m[0][1], matWorld.m[0][2] };
	obb.orientations[0] = NormalizeVec3(xAxis);

	Vector3 yAxis = { matWorld.m[1][0], matWorld.m[1][1], matWorld.m[1][2] };
	obb.orientations[1] = NormalizeVec3(yAxis);

	Vector3 zAxis = { matWorld.m[2][0], matWorld.m[2][1], matWorld.m[2][2] };
	obb.orientations[2] = NormalizeVec3(zAxis);

	return obb;
}