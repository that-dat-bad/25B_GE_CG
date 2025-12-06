#pragma once
#include "OBB.h"
#include <TDEngine.h>

class Needle {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="position">初期座標</param>

	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position, const TDEngine::Vector3 rotate);

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
	OBB GetOBB();
	// ワールドトランスフォームの取得
	TDEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }

private:
	// ワールド変換データ
	TDEngine::WorldTransform worldTransform_;
	// モデル
	TDEngine::Model* model_ = nullptr;

	// カメラ
	TDEngine::Camera* camera_ = nullptr;

	// キャラクターの当たり判定サイズ
	float width = 1.6f;
	float height = 1.6f;

	// 針の拡大率
	TDEngine::Vector3 upScale_ = {60.0f, 1.0f, 1.0f};

	// イージング用変数
	float t = 0.0f;
};