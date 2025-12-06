#include "Punch.h"
#include <cassert>

using namespace TDEngine;
using namespace MyMath;

// 簡易イージング
Vector3 EaseOutVec3(float t, const Vector3& start, const Vector3& end) {
	float t2 = 1.0f - (1.0f - t) * (1.0f - t);
	return {
		start.x + (end.x - start.x) * t2,
		start.y + (end.y - start.y) * t2,
		start.z + (end.z - start.z) * t2
	};
}
// EaseInVec3はNeedle.cppで定義したものと同じ（共通化推奨）
extern Vector3 EaseInVec3(float t, const Vector3& start, const Vector3& end);


void Punch::Initialize(Model* model, Camera* camera, const Vector3& position, int punched) {
	assert(model);
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation = position;

	// 初期位置調整
	worldTransform_.translation.x = 2.0f;
	worldTransform_.translation.y = 2.0f;

	isPunched = punched;
	punchedPosition = { position.x - 8.0f, position.y, position.z };
}

void Punch::Update() {
	t += 0.05f;

	if (t >= 1.0f) {
		t = 0.0f;
		if (isPunched) {
			isPunched = false;
			punchedPosition.x += 6.0f;
		}
		else {
			isPunched = true;
			punchedPosition.x -= 8.0f;
		}
	}
	else {
		if (isPunched) {
			worldTransform_.translation = EaseInVec3(t, worldTransform_.translation, punchedPosition);
		}
		else {
			worldTransform_.translation = EaseOutVec3(t, worldTransform_.translation, punchedPosition);
		}
	}

	worldTransform_.UpdateMatrix();
}

void Punch::Draw() {
	model_->Draw(worldTransform_, *camera_);
}

Vector3 Punch::GetWorldPosition() {
	return worldTransform_.translation;
}

AABB Punch::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = { worldPos.x - width / 2.0f, worldPos.y - height / 2.0f, worldPos.z - width / 2.0f };
	aabb.max = { worldPos.x + width / 2.0f, worldPos.y + height / 2.0f, worldPos.z + width / 2.0f };
	return aabb;
}