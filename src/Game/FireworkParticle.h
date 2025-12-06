#pragma once
#include "TDEngine.h"
#include <array>
#include <numbers>

class FireworkParticle {
private:
	// ワールド変換データ
	// (std::arrayで個数分持つ)
	std::array<TDEngine::WorldTransform, 16> worldTransforms_; // kNumParticles = 16

	// モデル
	TDEngine::Model* model_ = nullptr;

	// カメラ
	TDEngine::Camera* camera_ = nullptr;

	// パーティクルの個数
	static inline const uint32_t kNumParticles = 16;

	// 存続時間
	static inline const float kDuration = 1.0f;

	// 移動の速さ
	float speed = 2.0f;

	// 分割した一個分の角度
	static inline const float kAngleUnit = (std::numbers::pi_v<float> *4.0f) / kNumParticles;

	// 終了フラグ
	bool isFinished_ = false;

	// 経過時間カウント
	float counter_ = 0.0f;

	// 色の数値 (ObjectColorは廃止し、Vector4で管理)
	TDEngine::Vector4 color_;

public:
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position);
	void Update();
	void Draw();
	bool IsFinished() { return isFinished_; }
};