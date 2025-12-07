#include "TitleLogo.h"
#include <TDEngine.h>
#include <cassert>
using namespace TDEngine;

void TitleLogo::Initialize(
    TDEngine::Model* model, TDEngine::Camera* camera, const MyMath::Vector3& position) {
	// nullポインタチェック
	assert(model);

	// 引数をメンバ変数に記録
	model_ = model;
	camera_ = camera;

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
}

void TitleLogo::Update() {
	worldTransform_.translation_.y = sinf(theta_) * amplitude_;
	theta_ += static_cast<float>(M_PI) / 60.0f;
	worldTransform_.translation_.y += 2.0f;

	// 行列を定数バッファに移動
	worldTransform_.UpdateWorldMatrix(worldTransform_);
}

void TitleLogo::Draw() {
	// モデルの描画
	model_->Draw(worldTransform_, *camera_);
}