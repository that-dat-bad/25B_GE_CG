#pragma once
#include "TDEngine.h"
#include "AABB.h"

class Punch {
public:
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position, int punched);
	void Update();
	void Draw();

	void SetPosition(const TDEngine::Vector3& position) { worldTransform_.translation = position; }

	TDEngine::Vector3 GetWorldPosition();
	AABB GetAABB();

private:
	TDEngine::WorldTransform worldTransform_;
	TDEngine::Model* model_ = nullptr;
	TDEngine::Camera* camera_ = nullptr;

	float width = 1.6f;
	float height = 1.6f;
	float t = 0.0f;

	bool isPunched;
	TDEngine::Vector3 punchedPosition;
};