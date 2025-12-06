#pragma once
#include "AABB.h"
#include "EnemyDeathParticle.h"
#include <TDEngine.h>

class Player;
class Enemy;

class ChainBomb {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	// 範囲内のボムを爆発
	void ExplodeAround(const std::vector<ChainBomb*>& allChainBombs, float chainExplosionRadius);

	//爆発した時の処理
	void Explode();

	// プレイヤーとの衝突応答
	void OnCollision(const Player* player);
	// プレイヤーとのAABB
	AABB GetAABB();
	//敵との衝突応答
	void OnCollision(const Enemy* enemy);
	// 敵とのAABB
	AABB GetAABB(float size);

	const TDEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }
	TDEngine::Vector3 GetWorldPosition();

	const bool IsExplode() const { return isExplode_; }
	const bool IsDestroy() const { return isDestroy_; }
private:
	// カメラ
	TDEngine::Camera* camera_ = nullptr;

	// モデル
	TDEngine::Model* model_ = nullptr;
	// モデルサイズ
	TDEngine::Vector3 size_ = {2.0f, 2.0f, 2.0f};

	// ワールド変換データ
	TDEngine::WorldTransform worldTransform_;

	// 爆発フラグ
	bool isExplode_ = false;
	// 破壊フラグ
	bool isDestroy_ = false;

	static const int kExplodeFrame = 30;

	// 前のボムが爆発してから次のボムが爆発
	int explodeTimer_ = kExplodeFrame;

	// デスパーティクル
	EnemyDeathParticle* deathParticle_ = nullptr;

	// デスパーティクルのモデル
	TDEngine::Model* modelParticle_ = nullptr;
};
