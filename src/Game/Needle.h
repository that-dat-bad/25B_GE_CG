#pragma once
#include "OBB.h" // 必要ならTDEngine.hに統合してもOK
#include "TDEngine.h"

class Needle {
public:
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position, const TDEngine::Vector3 rotate);
	void Update();
	void Draw();

	TDEngine::Vector3 GetWorldPosition();
	OBB GetOBB();
	TDEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }

private:
	TDEngine::WorldTransform worldTransform_;
	TDEngine::Model* model_ = nullptr;
	TDEngine::Camera* camera_ = nullptr;

	float width = 1.6f;
	float height = 1.6f;

	TDEngine::Vector3 upScale_ = { 60.0f, 1.0f, 1.0f };
	float t = 0.0f;
};