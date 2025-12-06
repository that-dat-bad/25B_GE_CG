#include "Punch.h"
using namespace TDEngine;

void Punch::Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position, int punched) {
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

	// フラグを設定
	isPunched = punched;
	// パンチした後の位置を設定
	punchedPosition = {position.x - 8.0f, position.y, position.z};
}

void Punch::Update() {

	// tの値を増加
	t += 0.05f;

	if (t >= 1.0f) {

		// tの値をリセット
		t = 0.0f;

		if (isPunched) {
			// パンチが完了したらフラグを戻す
			isPunched = false;
			punchedPosition.x += 6.0f;
		} else {
			// パンチが完了したらフラグを立てる
			isPunched = true;
			punchedPosition.x -= 8.0f;
		}
	} else {
		if (isPunched) {
			// パンチを引っ込める
			worldTransform_.translation_ = worldTransform_.EaseIn(t, worldTransform_.translation_, punchedPosition);
		} else {
			// パンチする
			worldTransform_.translation_ = worldTransform_.EaseOut(t, worldTransform_.translation_, punchedPosition);
		}
	}

	// 行列を定数バッファに移動
	worldTransform_.UpdateWorldMatrix(worldTransform_);
}

void Punch::Draw() {
	// モデルの描画
	model_->Draw(worldTransform_, *camera_);
}

TDEngine::Vector3 Punch::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	worldPos = worldTransform_.translation_;
	return worldPos;
}

AABB Punch::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;

	aabb.min = {worldPos.x - width / 2.0f, worldPos.y - height / 2.0f, worldPos.z - width / 2.0f};
	aabb.max = {worldPos.x + width / 2.0f, worldPos.y + height / 2.0f, worldPos.z + width / 2.0f};

	return aabb;
}
