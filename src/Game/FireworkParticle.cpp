#include "FireworkParticle.h"
#include <algorithm>
#include <cmath>
#include "WorldTransform.h" // 拡張子とパスを修正

// --- ヘルパー関数 (TDEngineのWorldTransformにない機能) ---

// Z軸回転行列の作成
MyMath::Matrix4x4 MakeRotateZ(float angle) {
	MyMath::Matrix4x4 m = MyMath::Identity4x4();
	m.m[0][0] = std::cos(angle);
	m.m[0][1] = std::sin(angle);
	m.m[1][0] = -std::sin(angle);
	m.m[1][1] = std::cos(angle);
	return m;
}

// ベクトルと行列の掛け算 (回転適用など)
MyMath::Vector3 TransformNormalVec3(const MyMath::Vector3& v, const MyMath::Matrix4x4& m) {
	return {
		v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0],
		v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1],
		v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2]
	};
}

// ベクトルの加算
MyMath::Vector3 AddVec3(const MyMath::Vector3& a, const MyMath::Vector3& b) {
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

// --------------------------------------------------------

void FireworkParticle::Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const MyMath::Vector3& position) {
	assert(model);
	model_ = model;
	camera_ = camera;

	// 全パーティクルの初期化
	for (auto& worldTransform : worldTransforms_) {
		worldTransform.Initialize();
		worldTransform.translation = position;
		worldTransform.scale = { 1.0f, 1.0f, 1.0f };
	}

	// 初期カラー
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	counter_ = 0.0f;
	isFinished_ = false;
	speed = 2.0f;
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

	// 色の更新 (アルファ値を減衰)
	color_.w = std::clamp(1.0f - (counter_ / kDuration), 0.0f, 1.0f);

	for (uint32_t i = 0; i < kNumParticles; ++i) {

		if (i >= kNumParticles / 2) {
			speed = 0.01f;
		}

		// 基本となる速度ベクトル
		MyMath::Vector3 velocity = { speed, 0.0f, 0.0f };

		// 回転角を計算する
		float angle = kAngleUnit * i;

		// Z軸回りの回転行列
		MyMath::Matrix4x4 matrixRotation = MakeRotateZ(angle);

		// 基本ベクトルを回転させて速度ベクトルを得る
		velocity = TransformNormalVec3(velocity, matrixRotation);

		// 移動処理 (Add関数で加算)
		worldTransforms_[i].translation = AddVec3(worldTransforms_[i].translation, velocity);

		// モデルを傾ける
		worldTransforms_[i].rotation.z = angle;
	}

	// 行列更新
	for (auto& worldTransform : worldTransforms_) {
		worldTransform.UpdateMatrix();
	}
}

void FireworkParticle::Draw() {
	// 終了なら何もしない
	if (isFinished_) {
		return;
	}

	// アルファ値をモデルに適用
	model_->SetAlpha(color_.w);

	// 全パーティクルを描画
	for (auto& worldTransform : worldTransforms_) {
		model_->Draw(worldTransform, *camera_);
	}
}