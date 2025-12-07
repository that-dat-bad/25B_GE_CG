#pragma once
#include <TDEngine.h>
#include "WorldTransform.h"

class EnemyDeathParticle {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="position">初期座標</param>

	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const MyMath::Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>

	void Update();

	/// <summary>
	/// 描画
	/// </summary>

	void Draw();

private:
	// ワールド変換データ
	TDEngine::WorldTransform worldTransform_;
	// モデル
	TDEngine::Model* model_ = nullptr;
	// モデルのアルファ値
	float targetAlpha_ = 1.0f;
	TDEngine::ObjectColor objectColor_;
	MyMath::Vector4 color_;
	// アルファ値が上がり切ったかどうかのフラグ
	bool isAlphaMax_ = false;
	// カメラ
	TDEngine::Camera* camera_ = nullptr;

	// イージング用変数
	float t = 0.0f;
};
