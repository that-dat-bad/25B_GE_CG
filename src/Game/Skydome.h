#pragma once
#include "TDEngine.h"

class Skydome {
public:

	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera);
	void Update();
	void Draw();

private:
	TDEngine::WorldTransform worldTransform_;
	TDEngine::Model* model_ = nullptr;
	TDEngine::Camera* camera_ = nullptr;
};