#pragma once
#include "AABB.h"
#include <TDEngine.h>

class Thunder {
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
	// コリジョンフラグを取得
	bool IsCollisionDisabled() const { return isCollisionDisabled_; }

private:
	// ワールド変換データ
	TDEngine::WorldTransform worldTransform_;
	// モデル
	TDEngine::Model* model_ = nullptr;
	// モデルのアルファ値
	float targetAlpha_ = 1.0f;
	TDEngine::ObjectColor objectColor_;
	TDEngine::Vector4 color_;
	// アルファ値が上がり切ったかどうかのフラグ
	bool isAlphaMax_ = false;
	// コリジョン無効化フラグ
	bool isCollisionDisabled_ = false;
	// カメラ
	TDEngine::Camera* camera_ = nullptr;

	// キャラクターの当たり判定サイズ
	float width = 12.8f;
	float height = 80.0f;

	// イージング用変数
	float t = 0.0f;
};
