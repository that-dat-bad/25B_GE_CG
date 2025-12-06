#include "FireworkParticle.h"
#include <TDEngine.h>
#include <algorithm>
using namespace TDEngine;

void FireworkParticle::Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position) {

	worldTransform_.translation_.x = 2.0f;
	worldTransform_.translation_.y = 2.0f;

	// nullポインタチェック
	assert(model);

	// 引数をメンバ変数に記録
	model_ = model;
	camera_ = camera;

	// ワールドトランスフォームの初期化
	for (auto& worldTransform : worldTransforms_) {
		worldTransform.Initialize();
		worldTransform.translation_ = position;
	}

	objectColor_.Initialize();
	color_ = {1.0f, 1.0f, 1.0f, 1.0f};
}

void FireworkParticle::Update() {

	// 終了なら何もしない
	if (isFinished_) {
		return;
	}

	// カウンターを1フレーム分の秒数進める
	counter_ += 1.0f / 60.0f;

	// 存続時間の上限に達したら
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		// 終了扱いにする
		isFinished_ = true;
	}

	// 色の更新
	color_.w = std::clamp(1.0f - (counter_ / kDuration), 0.0f, 1.0f);
	// 色変更オブジェクトに色の数値を設定する
	objectColor_.SetColor(color_);

	for (uint32_t i = 0; i < kNumParticles; ++i) {

		if (i >= kNumParticles / 2)
		{
			speed = 0.01f;
		}
		
		// 基本となる速度ベクトル
		Vector3 velocity = {speed, 0.0f, 0.0f};
		// 回転角を計算する
		float angle = kAngleUnit * i;
		// Z軸回りの回転行列
		Matrix4x4 matrixRotation = worldTransform_.MakeRotateZMatrix(angle);
		// 基本ベクトルを回転させて速度ベクトルを得る
		velocity = worldTransform_.Transform(velocity, matrixRotation);
		// 移動処理
		worldTransforms_[i].translation_ = worldTransforms_[i].Add(worldTransforms_[i].translation_, velocity);
		// モデルを傾ける
		worldTransforms_[i].rotation_.z = angle;
	}

	for (WorldTransform& worldTransform : worldTransforms_) {
		// 行列を定数バッファに移動
		worldTransform.UpdateWorldMatrix(worldTransform);
	}
}

void FireworkParticle::Draw() {
	// 終了なら何もしない
	if (isFinished_) {
		return;
	}

	for (WorldTransform& worldTransform : worldTransforms_) {
		// モデルの描画
		model_->Draw(worldTransform, *camera_, &objectColor_);
	}
}
