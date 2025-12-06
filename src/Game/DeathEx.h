#pragma once
#include "TDEngine.h"

class DeathEx {
public:
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position, const TDEngine::Vector3 rotate);
	void Update();
	void Draw();

private:
	TDEngine::WorldTransform worldTransform_;
	TDEngine::Model* model_ = nullptr;
	float targetAlpha_ = 1.0f;

	// ObjectColor は削除し、Vector4のみで管理
	TDEngine::Vector4 color_;

	bool isAlphaMax_ = false;
	TDEngine::Camera* camera_ = nullptr;
	float t = 0.0f;
};