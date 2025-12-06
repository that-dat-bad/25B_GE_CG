#include "Beam.h"
using namespace TDEngine;


void Beam::Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position) {
	worldTransform_.translation_.x = 2.0f;
	worldTransform_.translation_.y = 2.0f;

	// nullポインタチェック
	assert(model);

	// 引数をメンバ変数に記録
	model_ = model;
	camera_ = camera;

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
}

void Beam::Update() {
	// 大きさを変更
	worldTransform_.scale_ = worldTransform_.Add(worldTransform_.scale_, upScale_);
	// 位置を調整
	worldTransform_.translation_ = worldTransform_.Add(worldTransform_.translation_, beamVelocity_);
	// 当たり判定サイズを調整
	width += (upScale_.x * 1.6f);
	// 行列を定数バッファに移動
	worldTransform_.UpdateWorldMatrix(worldTransform_);

}

void Beam::Draw() {
	// モデルの描画
	model_->Draw(worldTransform_, *camera_);
}

TDEngine::Vector3 Beam::GetWorldPosition() { 
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	worldPos = worldTransform_.translation_;
	return worldPos;
}

AABB Beam::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;

	aabb.min = {worldPos.x - width / 2.0f, worldPos.y - height / 2.0f, worldPos.z - width / 2.0f};
	aabb.max = {worldPos.x + width / 2.0f, worldPos.y + height / 2.0f, worldPos.z + width / 2.0f};

	return aabb;
}
