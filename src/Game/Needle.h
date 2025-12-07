#pragma once
#include "OBB.h"
#include "TDEngine.h"
#include"WorldTransform.h"

class Needle {
public:
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const MyMath::Vector3& position, const MyMath::Vector3 rotate);
	void Update();
	void Draw();

	MyMath::Vector3 GetWorldPosition();
	OBB GetOBB();
	WorldTransform& GetWorldTransform() { return worldTransform_; }

private:
	TDEngine::WorldTransform worldTransform_;
	TDEngine::Model* model_ = nullptr;
	TDEngine::Camera* camera_ = nullptr;

	float width = 1.6f;
	float height = 1.6f;

	MyMath::Vector3 upScale_ = { 60.0f, 1.0f, 1.0f };
	float t = 0.0f;
};