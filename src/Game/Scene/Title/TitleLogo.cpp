#include "TitleLogo.h"
#include <cassert>
#define _USE_MATH_DEFINES
#include <math.h>

void TitleLogo::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
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

	// 行列を定数バッファに移動
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	// 行列を定数バッファに移動
	worldTransform_.TransferMatrix();
}

void TitleLogo::Draw() {
	// モデルの描画
	model_->Draw(worldTransform_, *camera_);
}