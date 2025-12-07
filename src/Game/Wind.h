#pragma once
#include "AABB.h"
#include "TDEngine.h" // <TDEngine.h> -> "TDEngine.h"

class Wind {
public:
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const MyMath::Vector3& position);
	void Update();
	void Draw();

	MyMath::Vector3 GetWorldPosition();
	AABB GetAABB();

	MyMath::Vector3 GetVelocity() const {
		return {
			direction_.x * strength,
			direction_.y * strength,
			direction_.z * strength,
		};
	}


private:
	// カメラ
	TDEngine::Camera* camera_ = nullptr;

	// モデル
	TDEngine::Model* model_ = nullptr;

	// ワールド変換データ
	TDEngine::WorldTransform worldTransform_;

	// 風の向き
	MyMath::Vector3 direction_ = { -1.0f, 0.0f, 0.0f };
	// 風の強さ
	float strength = 1.0f;

	// 風の範囲
	MyMath::Vector3 range_ = { 48.0f, 6.0f, 2.0f };
};