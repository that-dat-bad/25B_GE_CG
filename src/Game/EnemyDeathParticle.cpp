#include "EnemyDeathParticle.h"

void EnemyDeathParticle::Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position) {
	
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

void EnemyDeathParticle::Update() {

	// tの値を増加
	t += 0.01f;

	if (isAlphaMax_) {

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
	color_.w = worldTransform_.EaseOutFloat(t, color_.w, targetAlpha_);
	objectColor_.SetColor(color_);
	// 行列を定数バッファに移動
	worldTransform_.UpdateWorldMatrix(worldTransform_);
}

void EnemyDeathParticle::Draw() {
	// モデルの描画
	model_->Draw(worldTransform_, *camera_, &objectColor_);
}
