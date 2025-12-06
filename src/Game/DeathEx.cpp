#include "DeathEx.h"

void DeathEx::Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position, const TDEngine::Vector3 rotate) {
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
	worldTransform_.rotation_ = rotate;
	worldTransform_.scale_ = {70.0f, 2.0f, 1.0f};

	// モデルのアルファ値を設定
	objectColor_.Initialize();
	color_ = {1.0f, 1.0f, 1.0f, 1.0f};
	color_.w = 0.0f;
}

void DeathEx::Update() {

	// tの値を増加
	t += 0.05f;

	if (isAlphaMax_) {
		// 処理をスキップ
		return;
	} else {
		if (color_.w >= 1.0f) {
			isAlphaMax_ = true;
		}
	}

	// モデルのアルファ値をイージング
	color_.w = worldTransform_.LerpFloat(color_.w, targetAlpha_, t);
	objectColor_.SetColor(color_);
	// 行列を定数バッファに移動
	worldTransform_.UpdateWorldMatrix(worldTransform_);
}

void DeathEx::Draw() {
	// モデルの描画
	model_->Draw(worldTransform_, *camera_, &objectColor_);
}
