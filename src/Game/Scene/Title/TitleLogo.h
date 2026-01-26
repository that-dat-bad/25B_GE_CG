#pragma once
#include "KamataEngine.h"

class TitleLogo {
private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;


public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();
	void SetPosition(const KamataEngine::Vector3& position) { worldTransform_.translation_ = position; }
};
