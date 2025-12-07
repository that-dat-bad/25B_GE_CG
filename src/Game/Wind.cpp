#include "Wind.h"

using namespace TDEngine;

void Wind::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(camera);
	camera_ = camera;

	assert(model);
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.translation = position; // translation_ -> translation
	worldTransform_.scale_ = { 0.5f, 0.5f, 0.5f }; // scale_ -> scale

	model_->SetAlpha(0.5f);

	worldTransform_.UpdateMatrix(); // UpdateWorldMatrix -> UpdateMatrix
}

void Wind::Update() {
	worldTransform_.UpdateMatrix();
}

void Wind::Draw() { model_->Draw(worldTransform_, *camera_); }

Vector3 Wind::GetWorldPosition() {
	Vector3 worldPos;
	// matWorld_ -> matWorld
	worldPos.x = worldTransform_.matWorld.m[3][0];
	worldPos.y = worldTransform_.matWorld.m[3][1];
	worldPos.z = worldTransform_.matWorld.m[3][2];
	return worldPos;
}

AABB Wind::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = { worldPos.x - range_.x / 2.0f, worldPos.y - range_.y / 2.0f, worldPos.z - range_.z / 2.0f };
	aabb.max = { worldPos.x + range_.x / 2.0f, worldPos.y + range_.y / 2.0f, worldPos.z + range_.z / 2.0f };
	return aabb;
}