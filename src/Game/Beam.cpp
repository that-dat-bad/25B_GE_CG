#include "Beam.h"
#include <cassert> // assert用

using namespace TDEngine;
using namespace MyMath; // AddVec3など用

// 簡易的な加算関数（TDEngineになければMyMath等から持ってくる）
Vector3 AddVec3(const Vector3& v1, const Vector3& v2) {
	return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

void Beam::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation = position;

	// 初期位置調整
	worldTransform_.translation.x = 2.0f;
	worldTransform_.translation.y = 2.0f;
}

void Beam::Update() {
	worldTransform_.scale_ = AddVec3(worldTransform_.scale_, upScale_);
	worldTransform_.translation = AddVec3(worldTransform_.translation, beamVelocity_);

	width += (upScale_.x * 1.6f);

	worldTransform_.UpdateMatrix();
}

void Beam::Draw() {
	model_->Draw(worldTransform_, *camera_);
}

Vector3 Beam::GetWorldPosition() {
	return worldTransform_.translation;
}

AABB Beam::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = { worldPos.x - width / 2.0f, worldPos.y - height / 2.0f, worldPos.z - width / 2.0f };
	aabb.max = { worldPos.x + width / 2.0f, worldPos.y + height / 2.0f, worldPos.z + width / 2.0f };
	return aabb;
}