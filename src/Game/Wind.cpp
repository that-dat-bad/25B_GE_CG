#include "Wind.h"

using namespace TDEngine;

void Wind::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(camera);
	camera_ = camera;

	assert(model);
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {0.5f, 0.5f, 0.5f};
	// モデルのアルファ値を設定
	model_->SetAlpha(0.5f);

	// ワールドトランスフォーム更新
	worldTransform_.UpdateWorldMatrix(worldTransform_);
}

void Wind::Update() {

	// ワールドトランスフォーム更新
	worldTransform_.UpdateWorldMatrix(worldTransform_);
}

void Wind::Draw() { model_->Draw(worldTransform_, *camera_); }

Vector3 Wind::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}
AABB Wind::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - range_.x / 2.0f, worldPos.y - range_.y / 2.0f, worldPos.z - range_.z / 2.0f};
	aabb.max = {worldPos.x + range_.x / 2.0f, worldPos.y + range_.y / 2.0f, worldPos.z + range_.z / 2.0f};

	return aabb;
}
