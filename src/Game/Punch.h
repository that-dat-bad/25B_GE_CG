#pragma once
#include <TDEngine.h>
#include "AABB.h"

class Punch {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="position">初期座標</param>

	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position, int punched);

	/// <summary>
	/// 更新
	/// </summary>

	void Update();

	/// <summary>
	/// 描画
	/// </summary>

	void Draw();

	// 位置を設定
	void SetPosition(const TDEngine::Vector3& position) { worldTransform_.translation_ = position; }

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
	float height = 1.6f;

	// イージング用変数
	float t = 0.0f;

	// パンチ済みフラグ
	bool isPunched;
	// パンチした後の拳の位置
	TDEngine::Vector3 punchedPosition;

};
