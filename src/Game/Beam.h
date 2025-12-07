#pragma once
#include "TDEngine.h"
#include "AABB.h"
#include"WorldTransform.h"

class Beam {
public:
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const MyMath::Vector3& position);
	void Update();
	void Draw();

	MyMath::Vector3 GetWorldPosition();
	AABB GetAABB();

private:
	TDEngine::WorldTransform worldTransform_;
	TDEngine::Model* model_ = nullptr;
	TDEngine::Camera* camera_ = nullptr;

	float width = 1.6f;
	float height = 3.2f;

	MyMath::Vector3 upScale_ = { 1.0f, 0.0f, 0.0f };
	MyMath::Vector3 beamVelocity_ = { -0.5f, 0.0f, 0.0f };
};