#include "DeathEx.h"

using namespace TDEngine;
using namespace MyMath; // 数学関数用

// 簡易イージング
float EaseOutFloatSimple(float t, float start, float end) {
	return start + (end - start) * (1.0f - (1.0f - t) * (1.0f - t));
}
// 簡易Lerp
float LerpFloatSimple(float a, float b, float t) {
	return a + (b - a) * t;
}

void DeathEx::Initialize(Model* model, Camera* camera, const Vector3& position, const Vector3 rotate) {
	worldTransform_.translation = position;

	assert(model);
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation = position;
	worldTransform_.rotation = rotate;
	worldTransform_.scale = { 70.0f, 2.0f, 1.0f };

	color_ = { 1.0f, 1.0f, 1.0f, 0.0f };
}

void DeathEx::Update() {
	t += 0.05f;

	if (isAlphaMax_) {
		return;
	}
	else {
		if (color_.w >= 1.0f) {
			isAlphaMax_ = true;
		}
	}

	// アルファ値をイージング
	color_.w = LerpFloatSimple(color_.w, targetAlpha_, t);

	worldTransform_.UpdateMatrix();
}

void DeathEx::Draw() {
	if (model_) {
		model_->SetAlpha(color_.w);
		model_->Draw(worldTransform_, *camera_);
	}
}