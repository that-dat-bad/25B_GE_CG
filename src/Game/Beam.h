#pragma once
#include <TDEngine.h>
#include "AABB.h"

class Beam {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="position">初期座標</param>

	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>

	void Update();

	/// <summary>
	/// 描画
	/// </summary>

	void Draw();

	// ワールド座標の取得
	TDEngine::Vector3 GetWorldPosition();
	// AABBを取得
	AABB GetAABB();
	
private:
	// ワールド変換データ
	TDEngine::WorldTransform worldTransform_;
	// モデル
	TDEngine::Model* model_ = nullptr;

	// カメラ
	TDEngine::Camera* camera_ = nullptr;

	// キャラクターの当たり判定サイズ
	float width = 1.6f;
	float height = 3.2f;

	// ビームの拡大率
	TDEngine::Vector3 upScale_ = {1.0f, 0.0f, 0.0f};

	// ビームの移動速度
	TDEngine::Vector3 beamVelocity_ = {-0.5f, 0.0f, 0.0f};
};
