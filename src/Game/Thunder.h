#pragma once
#include "AABB.h"
#include "TDEngine.h"
#include"WorldTransform.h"

class Thunder {
public:
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const MyMath::Vector3& position);
	void Update();
	void Draw();

	MyMath::Vector3 GetWorldPosition();
	AABB GetAABB();
	bool IsCollisionDisabled() const { return isCollisionDisabled_; }

private:
	TDEngine::WorldTransform worldTransform_;
	TDEngine::Model* model_ = nullptr;

	MyMath::Vector4 color_;

	bool isAlphaMax_ = false;
	bool isCollisionDisabled_ = false;
	TDEngine::Camera* camera_ = nullptr;

	float width = 12.8f;
	float height = 80.0f;
	float t = 0.0f;
};