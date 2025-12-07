#pragma once
#include "TDEngine.h"
#include "AABB.h"
#include"WorldTransform.h"
class Punch {
public:
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const MyMath::Vector3& position, int punched);
	void Update();
	void Draw();

	void SetPosition(const MyMath::Vector3& position) { worldTransform_.translation_ = position; }

	MyMath::Vector3 GetWorldPosition();
	AABB GetAABB();

private:
	TDEngine::WorldTransform worldTransform_;
	TDEngine::Model* model_ = nullptr;
	TDEngine::Camera* camera_ = nullptr;

	float width = 1.6f;
	float height = 1.6f;
	float t = 0.0f;

	bool isPunched;
	MyMath::Vector3 punchedPosition;
};