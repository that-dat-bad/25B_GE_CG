#include "Thunder.h"
#include <cassert>

using namespace TDEngine;
using namespace MyMath;

// 簡易イージング (float版)
float EaseOutFloatSimple(float t, float start, float end) {
	return start + (end - start) * (1.0f - (1.0f - t) * (1.0f - t));
}

void Thunder::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation = position;

	// 初期位置調整
	worldTransform_.translation.x = 2.0f;
	worldTransform_.translation.y = 2.0f;

	// アルファ初期化
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void Thunder::Update() {
	t += 0.01f;

	if (isAlphaMax_) {
		if (color_.w <= 0.5f) {
			isCollisionDisabled_ = true;
		}
		if (t >= 1.0f) {
			return;
		}
	}
	else {
		if (color_.w >= 1.0f) {
			isAlphaMax_ = true;
			targetAlpha_ = 0.0f;
			t = 0.0f;
		}
	}

	// アルファ値をイージング
	color_.w = EaseOutFloatSimple(t, color_.w, targetAlpha_);

	worldTransform_.UpdateMatrix();
}

void Thunder::Draw() {
	// モデルのアルファ値をセットして描画
	if (model_) {
		model_->SetAlpha(color_.w);
		model_->Draw(worldTransform_, *camera_);
	}
}

Vector3 Thunder::GetWorldPosition() {
	return worldTransform_.translation;
}

AABB Thunder::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = { worldPos.x - width / 2.0f, worldPos.y - height / 2.0f, worldPos.z - width / 2.0f };
	aabb.max = { worldPos.x + width / 2.0f, worldPos.y + height / 2.0f, worldPos.z + width / 2.0f };
	return aabb;
}