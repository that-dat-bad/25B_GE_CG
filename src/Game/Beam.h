#pragma once
#include "TDEngine.h"
#include "AABB.h"

class Beam {
public:
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position);
	void Update();
	void Draw();

	TDEngine::Vector3 GetWorldPosition();
	AABB GetAABB();

private:
	TDEngine::WorldTransform worldTransform_;
	TDEngine::Model* model_ = nullptr;
	TDEngine::Camera* camera_ = nullptr;

	float width = 1.6f;
	float height = 3.2f;

	TDEngine::Vector3 upScale_ = { 1.0f, 0.0f, 0.0f };
	TDEngine::Vector3 beamVelocity_ = { -0.5f, 0.0f, 0.0f };
};