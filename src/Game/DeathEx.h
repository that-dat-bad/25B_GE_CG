#pragma once
#include "TDEngine.h"
#include <WorldTransform.cpp>

class DeathEx {
public:
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const MyMath::Vector3& position, const MyMath::Vector3 rotate);
	void Update();
	void Draw();

private:
	WorldTransform worldTransform_;
	TDEngine::Model* model_ = nullptr;
	float targetAlpha_ = 1.0f;

	
	MyMath::Vector4 color_;

	bool isAlphaMax_ = false;
	TDEngine::Camera* camera_ = nullptr;
	float t = 0.0f;
};