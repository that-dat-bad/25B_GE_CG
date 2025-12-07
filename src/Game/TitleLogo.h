#pragma once
#pragma once
#include <TDEngine.h>
#define _USE_MATH_DEFINES
#include <math.h>

/// <summary>
/// タイトルシーン
/// </summary>

class TitleLogo {
private:
	// ワールド変換データ
	TDEngine::WorldTransform worldTransform_;

	// モデル
	TDEngine::Model* model_ = nullptr;

	// カメラ
	TDEngine::Camera* camera_ = nullptr;

	// タイトルロゴを動かす用の変数
	float amplitude_ = 0.5f;
	float theta_ = 0.0f;

public:
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const MyMath::Vector3& position);
	void Update();
	void Draw();
};