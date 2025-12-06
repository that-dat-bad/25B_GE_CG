#include "Thunder.h"
using namespace TDEngine;

void Thunder::Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position) {
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
	// モデルのアルファ値を設定
	objectColor_.Initialize();
	color_ = {1.0f, 1.0f, 1.0f, 1.0f};
}

void Thunder::Update() {

	// tの値を増加
	t += 0.01f;

	if (isAlphaMax_) {

		if (color_.w <= 0.5f) {
			// コリジョン無効化フラグを立てる
			isCollisionDisabled_ = true;
		}

		if (t >= 1.0f) {
			// 処理をスキップ
			return;
		}

	} else {
		if (color_.w >= 1.0f) {
			isAlphaMax_ = true;
			// ターゲットアルファ値を0に設定
			targetAlpha_ = 0.0f;
			// tをリセット
			t = 0.0f;
		}
	}

	// モデルのアルファ値をイージング
	color_.w = worldTransform_.EaseOutFloat(t,color_.w,targetAlpha_);
	objectColor_.SetColor(color_);
	// 行列を定数バッファに移動
	worldTransform_.UpdateWorldMatrix(worldTransform_);
}

void Thunder::Draw() {
	// モデルの描画
	model_->Draw(worldTransform_, *camera_, &objectColor_);
}

TDEngine::Vector3 Thunder::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	worldPos = worldTransform_.translation_;
	return worldPos;
}

AABB Thunder::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;

	aabb.min = {worldPos.x - width / 2.0f, worldPos.y - height / 2.0f, worldPos.z - width / 2.0f};
	aabb.max = {worldPos.x + width / 2.0f, worldPos.y + height / 2.0f, worldPos.z + width / 2.0f};

	return aabb;
}
