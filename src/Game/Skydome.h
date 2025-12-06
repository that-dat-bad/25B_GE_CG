#pragma once
#include <3d/Model.h>
#include <TDEngine.h>

class Skydome {

public:
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera);
	void Update();
	void Draw();

private:
	// ワールド変換データ
	TDEngine::WorldTransform worldTransform_;

	// モデル
	TDEngine::Model* model_ = nullptr;

	// カメラ
	TDEngine::Camera* camera_ = nullptr;
};